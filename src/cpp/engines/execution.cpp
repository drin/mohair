// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "../headers/faodel.hpp"


// ------------------------------
// Functions

using std::vector;


/**
 * A function that takes a serialized substrait plan as a binary string, executes it, then
 * puts the results in `ext_ldo`.
 */
FaoStatus ExecuteSubstrait(        FaoBucket       b
                            ,const KelpKey         k
                            ,const string         &args
                            ,map<KelpKey, LunaDO>  fado_map
                            ,LunaDO               *ext_ldo) {
  // NOTE: in fado-examples, b and k are unused (we just iterate over fado_map)

  // TODO: define a data provider that uses fado_map
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

  // Check if the substrait plan was successfully parsed
  if (not result_plan.ok()) {
    // TODO: return an appropriate error
    return 0;
  }

  // Get the root Declaration of the Result<PlanInfo> for Acero execution
  // `DeclarationToTable` takes a Declaration, creates an ExecPlan, executes it, and
  // returns the results in an appropriate format (DeclarationToTable returns a Table).
  // NOTE: there are async versions of `DeclarationTo<>`
  QueryOptions plan_opts;
  const Declaration         &plan_root     = result_plan->root.declaration;
  Result<shared_ptr<Table>>  query_results = arrow::acero::DeclarationToTable(
    plan_root, std::move(plan_opts)
  );

  // query_result
  // TODO: double check this portion
  ArrowDO fado { query_results->get() };
  // TODO: fado.SetObjectStats(kAllExecuted);
  *ext_ldo = fado.ExportDataObject();

  return KELPIE_OK;
}
