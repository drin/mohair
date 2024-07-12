// ------------------------------
// License
//
// Copyright 2024 Aldrin Montana
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

// >> Configuration-based macros
#include "../mohair-config.hpp"

#include "service_duckdb.hpp"


// ------------------------------
// Functions

namespace mohair::services {

  Status StartMohairDuckDB() {
    unique_ptr<FlightServerBase> duckdb_service = std::make_unique<DuckDBService>();
    return StartService(duckdb_service);
  }

} // namespace: mohair::services


// ------------------------------
// Classes and Methods

namespace mohair::services {

  // >> DuckDBService

  //   |> Constructors
  DuckDBService::DuckDBService() {
    duck_if = mohair::adapters::DuckDBForMem();
  }

  DuckDBService::DuckDBService(fs::path db_fpath) {
    duck_if = mohair::adapters::DuckDBForFile(db_fpath);
  }

  //    |> Custom Flight API
  Status
  DuckDBService::ActionQuery( [[maybe_unused]] const ServerCallContext&  context
                             ,                 const shared_ptr<Buffer>  plan_msg
                             ,                 unique_ptr<ResultStream>* result) {
    // convert message to string type
    string plan_data = plan_msg->ToString();

    // convert substrait plan to duckdb plan
    auto queryid_result = duck_if->SubstraitPlanMessage(plan_data);
    if (not queryid_result.ok()) {
      std::cerr << "Failed to translate substrait query plan:" << std::endl
                << "\t" << queryid_result.status().message()   << std::endl
      ;

      return Status::Invalid("Failed to translate substrait plan message");
    }

    // the result of conversion is a query ID
    int    query_id     = *queryid_result;
    string query_ticket = std::to_string(query_id);

    // write the query ID to the `ResultStream` as a usable ticket
    auto result_buffer = Buffer::FromString(query_ticket);
    *result = std::make_unique<SimpleResultStream>(
      vector<FlightResult> { FlightResult { result_buffer } }
    );

    // execute the query and return the result (or OK)
    ARROW_RETURN_NOT_OK(duck_if->ExecuteRelation(query_id));
    return Status::OK();
  }

  //    |> Standard Flight API
  Status
  DuckDBService::DoGet( [[maybe_unused]] const ServerCallContext&      context
                       ,const Ticket&                 request
                       ,[[maybe_unused]] unique_ptr<FlightDataStream>* stream) {
    int   query_id    = std::stoi(request.ticket);
    auto& duck_result = duck_if->GetResult(query_id);

    // NOTE: this is just for debugging purposes until we convert
    std::cout << "Accessed results:" << std::endl;
    ARROW_RETURN_NOT_OK(mohair::adapters::PrintQueryResults(duck_result));

    // TODO: convert the results from duckdb to arrow
    return Status::NotImplemented("TODO: query service");
  }

} // namespace: mohair::services
