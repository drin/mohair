// ------------------------------
// License
//
// Copyright 2023 Aldrin Montana
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


// ------------------------------
// Dependencies

#include "engines/apidep_acero.hpp"


// ------------------------------
// Classes and Functions

namespace skytether::adapters {

  //! Execute an Acero plan (arrow::engine::PlanInfo) using arrow::acero::DeclarationToTable.
  /*
  Result<shared_ptr<Table>> ExecutePlan(PlanInfo &acero_plan) {
    QueryOptions default_planopts;
    const Declaration &plan_root = acero_plan.root.declaration;

    return arrow::acero::DeclarationToTable(plan_root, std::move(default_planopts));
  }
  */

  /* TODO: some code to eventually convert into an interface to Acero

    //! A lambda that implements the TableProvider interface:
        - tname  : a vector of strings that collectively make up a single table name
        - tschema: an expected schema of the table, only used with early binding
      auto tprovider = []( const vector<string> &tname
                          ,const Schema         &) -> Result<Declaration> {
        // concatenate into a single table which we wrap in a Declaration
        // the Declaration essentially represents the data source for a scan node
        ARROW_ASSIGN_OR_RAISE(auto fado_as_table, arrow::ConcatenateTables(fado_chunks));
        auto options = arrow::acero::TableSourceNodeOptions(std::move(fado_as_table));

        return Declaration(
           "table_source"
          ,TableSourceNodeOptions { std::move(fado_as_table) }
          ,requested_tname
        );
      }

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

  */

} // mohair::adapters
