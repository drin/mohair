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
#include "skytether-config.hpp"

#include "service_duckdb.hpp"


// ------------------------------
// Classes and Methods

// >> DuckDBService implementations
namespace skytether::services {

  // Constructors
  DuckDBService::DuckDBService(ShutdownCallback* cb_custom)
    : EngineService(cb_custom) {
    engine = skytether::adapters::DuckDBForMem();
  }

  DuckDBService::DuckDBService(ShutdownCallback* cb_custom, fs::path db_fpath)
    : EngineService(cb_custom) {
    engine = skytether::adapters::DuckDBForFile(db_fpath);
  }

  DuckDBService::DuckDBService()
    : DuckDBService::DuckDBService(nullptr) {}

  DuckDBService::DuckDBService(fs::path db_fpath)
    : DuckDBService::DuckDBService(nullptr, db_fpath) {}

  // Custom Flight API
  Status
  DuckDBService::DoPlanPushdown( [[maybe_unused]] const ServerCallContext&  context
                                ,                 const shared_ptr<Buffer>  plan_msg
                                ,                 unique_ptr<ResultStream>* result) {
    // convert message to string type
    SkytetherDebugMsg("Received query request");
    string plan_data = plan_msg->ToString();

    // convert substrait plan to duckdb plan
    SkytetherDebugMsg("Passing query plan to query engine");
    int context_id = engine->ExecContextForSubstrait(plan_data);
    if (not context_id) { return Status::Invalid("Failed to translate substrait"); }

    // write the query ID to the `ResultStream` as a usable ticket
    SkytetherDebugMsg("Preparing ticket for response data");
    SkytetherTicket query_ticket { context_id };

    SkytetherDebugMsg("Responding with ticket: " << std::to_string(context_id));
    *result = std::make_unique<SimpleResultStream>(
      vector<FlightResult> { FlightResult { query_ticket.ToBuffer() } }
    );

    // execute the query and return the result (or OK)
    SkytetherDebugMsg("Executing query plan");
    ARROW_RETURN_NOT_OK(engine->ExecuteRelation(context_id));
    return Status::OK();
  }

  Status
  DuckDBService::DoPlanExecution( [[maybe_unused]] const ServerCallContext&  context
                                 ,[[maybe_unused]] const shared_ptr<Buffer>  plan_msg
                                 ,[[maybe_unused]] unique_ptr<ResultStream>* result) {
    return Status::NotImplemented("TODO: query service");
  }


  // Standard Flight API
  Status
  DuckDBService::DoGet( [[maybe_unused]] const ServerCallContext&      context
                       ,                 const Ticket&                 request
                       ,[[maybe_unused]] unique_ptr<FlightDataStream>* result_stream) {
    int query_id = std::stoi(request.ticket);
    SkytetherDebugMsg("Get request for ticket: " << request.ticket);

    shared_ptr<RecordBatchReader> resultset_reader { engine->GetResultSet(query_id) };
    if (resultset_reader == nullptr) {
      stringstream err_msg;
      err_msg << "Unknown ticket: " << request.ticket;
      return Status::Invalid(err_msg.str());
    }

    *result_stream = std::make_unique<RecordBatchStream>(resultset_reader);
    return Status::OK();
  }

} // namespace: skytether::services

