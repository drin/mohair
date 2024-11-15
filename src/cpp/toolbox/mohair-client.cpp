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

// >> Standard libs
#include <unistd.h>

// >> Internal
#include "mohair_cli.hpp"
#include "../services/ticket_mohair.hpp"
#include "../services/service_mohair.hpp"


// ------------------------------
// Macros and Type Aliases

// >> Types
using mohair::Status;
using mohair::Buffer;

using mohair::services::Location;
using mohair::services::ResultStream;
using mohair::services::FlightStreamReader;

using mohair::services::FlightStreamChunk;

using mohair::services::MohairClient;
using mohair::services::MohairTicket;

// >> Functions
using mohair::cli::ParseArgLocationUri;


// ------------------------------
// Structs and Classes

struct ClientActions {
  bool should_shutdown;
  bool should_dereg;

  Location           service_loc;
  Location           target_loc;
  shared_ptr<Buffer> request_payload;

  ClientActions()
    :  should_shutdown(false)
      ,should_dereg(false)
      ,service_loc()
      ,request_payload(nullptr)
  {}


  // >> "System Interface"

  //! Submits a single query plan then validates the response is a single ticket
  Status ExecutePlan(MohairClient& client_conn, MohairTicket* out_ticket) {
    ARROW_ASSIGN_OR_RAISE(
       unique_ptr<ResultStream> ticket_response
      ,client_conn.SendPlanPushdown(request_payload)
    );

    ARROW_ASSIGN_OR_RAISE(
       *out_ticket
      ,mohair::services::ExpectResultFromQuery(std::move(ticket_response))
    );

    return Status::OK();
  }

  //! Submits a single ticket to request query results, then prints the results
  Status GetResults(MohairClient& client_conn, MohairTicket& query_ticket) {
    constexpr int count_peeksize = 5;

    ARROW_ASSIGN_OR_RAISE(
       unique_ptr<FlightStreamReader> result_reader
      ,client_conn.GetQueryResults(query_ticket)
    );

    ARROW_ASSIGN_OR_RAISE(FlightStreamChunk result_chunk, result_reader->Next());
    for (int peek_ndx = 0; result_chunk.data and peek_ndx < count_peeksize; ++peek_ndx) {
      mohair::PrintRecordBatch(result_chunk.data, 0, 10);
      ARROW_ASSIGN_OR_RAISE(FlightStreamChunk result_chunk, result_reader->Next());
    }

    return Status::OK();
  }


  // >> "Application Interface"

  //! Executes a query by submitting the query plan then fetching the results.
  Status ExecuteQuery(MohairClient& client_conn) {
    MohairDebugMsg("Sending query request");
    if (request_payload == nullptr) { return Status::Invalid("Missing query plan"); }

    MohairTicket query_ticket;

    ARROW_RETURN_NOT_OK(ExecutePlan(client_conn, &query_ticket));
    ARROW_RETURN_NOT_OK( GetResults(client_conn,  query_ticket));

    std::cout << "Query complete" << std::endl;
    return Status::OK();
  }


  // >> Public entry point

  //! The entry point for the `ClientActions` struct to send all user requests.
  int SendRequests() {
    // Create and connect a FlightClient
    auto client_conn = MohairClient::ForLocation(service_loc);
    if (client_conn == nullptr) { return ERRCODE_CONN_CLIENT; }

    // Handle each action depending on what actions were set
    if (request_payload != nullptr) {
      auto status_query = ExecuteQuery(*client_conn);
      if (not status_query.ok()) {
        mohair::PrintError("Unable to execute query plan", status_query);
        return ERRCODE_API_QUERY;
      }
    }

    if (should_shutdown) {
      auto result_shutdown = client_conn->SendSignalShutdown();
      if (not result_shutdown.ok()) {
        mohair::PrintError("Unable to shutdown service", result_shutdown.status());
        return ERRCODE_API_SHUTDOWN;
      }
    }

    if (should_dereg) {
      auto result_dereg = client_conn->SendDeactivation(target_loc);
      if (not result_dereg.ok()) {
        mohair::PrintError("Unable to deregister service", result_dereg.status());
        return ERRCODE_API_DEREGISTER;
      }
    }

    return 0;
  }
};


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "mohair-client"
              << " -l service-location-uri"
              << " -q path-to-plan-file"
              << " [-d location-to-deregister]"
              << " [-k]"
              << " [-h]"
              << std::endl
              << "-k sends shutdown request to service location"
              << std::endl
              << "-d sends request to de-register target location from metadata service"
              << std::endl
              << std::endl
    ;

    return 1;
}


// ------------------------------
// Main Logic

int main(int argc, char **argv) {
  ClientActions client_actions;

  // Parse each argument and internalize the provided option
  constexpr char  is_done_parsing = -1;
  const     char* opt_template    = "l:q:d:kh";
  char            parsed_opt;
  int             errcode_cli;

  while ((parsed_opt = (char) getopt(argc, argv, opt_template)) != is_done_parsing) {
    switch (parsed_opt) {

      case 'h': { return PrintHelp(); }

      case 'l': {
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.service_loc));
        MohairCheckErrCode(errcode_cli, "Failed to parse service location");
        break;
      }

      case 'q': {
        auto result_buffer = mohair::BufferFromFile(optarg);
        if (not result_buffer.ok()) {
          mohair::PrintError("Unable to read plan file", result_buffer.status());
          return ERRCODE_FILE_PARSE;
        }
        client_actions.request_payload = result_buffer.ValueOrDie();
        break;
      }

      case 'k': {
        client_actions.should_shutdown = true;
        break;
      }

      case 'd': {
        client_actions.should_dereg = true;
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.target_loc));
        MohairCheckErrCode(errcode_cli, "Failed to parse target location");
        break;
      }

      default: { break; }
    }
  }

  return client_actions.SendRequests();
}
