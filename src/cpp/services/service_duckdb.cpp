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

  // Create a flight server that runs until it dies
  int StartMohairDuckDB(const Location& srv_loc) {
    unique_ptr<FlightServerBase> duckdb_service = std::make_unique<DuckDBService>();

    auto status_service = StartService(duckdb_service.get(), srv_loc);
    if (not status_service.ok()) {
      mohair::PrintError("Error running cs-engine [duckdb]", status_service);
      return ERRCODE_START_SRV;
    }

    return 0;
  }

  int StartMohairDuckDB(const Location& srv_loc, ShutdownCallback* fn_shutdown) {
    auto duckdb_service = std::make_unique<DuckDBService>();
    duckdb_service->cb_shutdown = fn_shutdown;

    FlightServerBase* service = dynamic_cast<FlightServerBase*>(duckdb_service.get());
    auto status_service = StartService(service, srv_loc);
    if (not status_service.ok()) {
      mohair::PrintError("Error running cs-engine [duckdb]", status_service);
      return ERRCODE_START_SRV;
    }

    return 0;
  }

  int StartMohairDuckDB(const ServiceConfig& srv_cfg, ShutdownCallback* fn_shutdown) {
    auto duckdb_service = std::make_unique<DuckDBService>();
    duckdb_service->service_cfg = srv_cfg;
    duckdb_service->cb_shutdown = fn_shutdown;

    auto result_loc = Location::Parse(srv_cfg.service_location());
    if (not result_loc.ok()) {
      mohair::PrintError("Error parsing location from config", result_loc.status());
      return ERRCODE_CREATE_LOC;
    }

    FlightServerBase* service = dynamic_cast<FlightServerBase*>(duckdb_service.get());
    auto status_service = StartService(service, std::move(result_loc).ValueOrDie());
    if (not status_service.ok()) {
      mohair::PrintError("Error running cs-engine [duckdb]", status_service);
      return ERRCODE_START_SRV;
    }

    return 0;
  }

} // namespace: mohair::services


// ------------------------------
// Classes and Methods

// >> DuckDBService implementations
namespace mohair::services {

  // Constructors
  DuckDBService::DuckDBService() {
    duck_if = mohair::adapters::DuckDBForMem();
  }

  DuckDBService::DuckDBService(fs::path db_fpath) {
    duck_if = mohair::adapters::DuckDBForFile(db_fpath);
  }


  // Custom Flight API
  Status
  DuckDBService::ActionQuery( [[maybe_unused]] const ServerCallContext&  context
                             ,                 const shared_ptr<Buffer>  plan_msg
                             ,                 unique_ptr<ResultStream>* result) {
    // convert message to string type
    string plan_data = plan_msg->ToString();

    // convert substrait plan to duckdb plan
    int context_id = duck_if->ExecContextForSubstrait(plan_data);
    if (not context_id) { return Status::Invalid("Failed to translate substrait"); }

    // write the query ID to the `ResultStream` as a usable ticket
    string query_ticket { std::to_string(context_id) };
    auto ticket_buffer = Buffer::FromString(query_ticket);
    *result = std::make_unique<SimpleResultStream>(
      vector<FlightResult> { FlightResult { ticket_buffer } }
    );

    // execute the query and return the result (or OK)
    ARROW_RETURN_NOT_OK(duck_if->ExecuteRelation(context_id));
    return Status::OK();
  }


  // Standard Flight API
  Status
  DuckDBService::DoGet( [[maybe_unused]] const ServerCallContext&      context
                       ,                 const Ticket&                 request
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

