// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "adapter_faodel.hpp"


// ------------------------------
// Functions

namespace mohair::adapters {

  // >> Convenience functions for interfacing with Acero

  /**
   * Executes an Acero plan (arrow::engine::PlanInfo) using arrow::acero::DeclarationToTable.
   *
   * DeclarationToTable takes a Declaration, then creates and executes an ExecPlan. There
   * are async versions (with an async suffix) and the "ToTable" suffix indicates that the
   * results are returned as an arrow::Table.
   */
  Result<shared_ptr<Table>> ExecutePlan(PlanInfo &acero_plan) {
    QueryOptions default_planopts;
    const Declaration &plan_root = acero_plan.root.declaration;

    return arrow::acero::DeclarationToTable(plan_root, std::move(default_planopts));
  }

  FaoStatus FaodelStatusFromArrowStatus(const Status arrow_status) {
    if (arrow_status.ok())              { return kelpie::KELPIE_OK;     }
    else if (arrow_status.IsInvalid())  { return kelpie::KELPIE_EINVAL; }
    else if (arrow_status.IsKeyError()) { return kelpie::KELPIE_ENOENT; }
    else if (arrow_status.IsIOError())  { return kelpie::KELPIE_EIO;    }
    else {
      // any other error is still an error, but kelpie doesn't have a mapping for it
      return kelpie::KELPIE_TODO;
    }
  }

  // >> Functions for interfacing with execution engines from compute frameworks

  /**
   * Convenience higher-order function that returns a `NamedTableProvider`.
   *
   * A NamedTableProvider is a function that takes 2 parameters and returns a
   * `Declaration`.
   *
   * The 2 parameters describe a requested table:
   *  - a decomposed table name (vector<string> that represents a single table)
   *  - a table schema
   *
   * The resulting Declaration describes a Source Node for a query plan.
   */
  NamedTableProvider ProviderForFadoMap(map<KelpKey, LunaDO> &fado_map) {
    /**
     * A lambda that captures the given fado_map by reference and takes two parameters:
     *  - tname  : a vector of strings that collectively make up a single table name
     *  - tschema: an expected schema of the table, only used with early binding
     */
    return [&fado_map]( const vector<string> &tname
                       ,const Schema         &) -> Result<Declaration> {

      // gather the parts of the table name
      auto requested_tname = mohair::JoinStr(tname, ".");

      // lookup the name in the fado_map, store entry into a structured binding
      if (fado_map.count(requested_tname) == 0) {
        return arrow::Status::KeyError(
           "Fado table provider could not find table: [", requested_tname, "]"
        );
      }

      // wrap the requested lunasa data object in a faodel arrow data object
      const auto& ldo = fado_map[requested_tname];
      ArrowDO fado { ldo };

      vector<shared_ptr<Table>> fado_chunks;
      fado_chunks.reserve(fado.NumberOfTables());

      // extract each table from the data object (also called chunks)
      for (int table_ndx = 0; table_ndx < fado.NumberOfTables(); ++table_ndx) {
        ARROW_ASSIGN_OR_RAISE(auto fado_chunk, fado.ExtractTable(table_ndx));
        fado_chunks.push_back(fado_chunk);
      }

      // concatenate into a single table which we wrap in a Declaration
      // the Declaration essentially represents the data source for a scan node
      ARROW_ASSIGN_OR_RAISE(auto fado_as_table, arrow::ConcatenateTables(fado_chunks));
      auto options = arrow::acero::TableSourceNodeOptions(std::move(fado_as_table));

      return Declaration(
         "table_source"
        ,TableSourceNodeOptions { std::move(fado_as_table) }
        ,requested_tname
      );
    };
  }


  /**
   * A function that takes a serialized substrait plan as a binary string, executes it, then
   * puts the results in `ext_ldo`.
   *
   * NOTE: based on an example, FaoBucket and KelpKey are unused, so we will figure that
   * out later.
   */
  FaoStatus ExecuteSubstrait(        FaoBucket    /* b */
                              ,const KelpKey      /* k */
                              ,const string         &args
                              ,map<KelpKey, LunaDO>  fado_map
                              ,LunaDO               *ext_ldo) {
    // ConversionOptions controls translation of a plan between Substrait and Acero.
    // The only one we're interested in for now is to customize handling of named tables.
    ConversionOptions conv_opts;
    conv_opts.named_table_provider = mohair::adapters::ProviderForFadoMap(fado_map);

    // Create a buffer using a copy of `args` (protobuf serialized to a binary string)
    auto         serialized_plan  = Buffer::FromString(string { args });
    auto         default_registry = arrow::engine::default_extension_id_registry();

    // Parse substrait plan into a Result<PlanInfo> and stash the constructed ExtensionSet
    ExtensionSet acero_ext_set;
    auto result_plan = arrow::engine::DeserializePlan(
      *serialized_plan, default_registry, &acero_ext_set, conv_opts
    );

    if (not result_plan.ok()) {
      mohair::PrintError("Error when translating substrait to acero:", result_plan.status());
      return FaodelStatusFromArrowStatus(result_plan.status());
    }

    // Get the root Declaration of the Result<PlanInfo> for Acero execution
    auto query_results = ExecutePlan(*result_plan);
    if (not query_results.ok()) {
      mohair::PrintError("Error when executing acero plan:", query_results.status());
      return FaodelStatusFromArrowStatus(query_results.status());
    }

    // Wrap the query result in a fado, set its status, then export to the output argument
    ArrowDO fado { *query_results };
    fado.SetObjectStatus(kelpie::KELPIE_OK);
    *ext_ldo = fado.ExportDataObject();

    return kelpie::KELPIE_OK;
  }

} // namespace: mohair::adapters
