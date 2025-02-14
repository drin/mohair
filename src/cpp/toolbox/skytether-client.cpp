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
#include "skytether_cli.hpp"
#include "services/types.hpp"
#include "services/service_skytether.hpp"


// ------------------------------
// Macros and Type Aliases

// >> Standard types
using std::unique_ptr;
using std::shared_ptr;

// >> Types
using skytether::Status;
using skytether::Buffer;

using skytether::services::Location;
using skytether::services::ResultStream;
using skytether::services::FlightStreamReader;

using skytether::services::FlightStreamChunk;

using skytether::services::SkytetherClient;
using skytether::services::SkytetherTicket;

using mohair::PlanMessage;
using mohair::Plan;

// >> Functions
using skytether::cli::ParseArgLocationUri;


// ------------------------------
// Structs and Classes

struct ClientActions {
  bool should_shutdown { false };
  bool should_dereg    { false };

  Location                service_loc;
  Location                target_loc;
  unique_ptr<PlanMessage> query_plan;


  // >> "Application Interface"
  //! Executes a query by submitting the query plan then fetching the results.
  Status ExecuteQuery(SkytetherClient& client_conn) {
    SkytetherDebugMsg("Sending query request");
    ARROW_ASSIGN_OR_RAISE(
       unique_ptr<Plan> pushback_plan
      ,client_conn.DelegatePlan(*query_plan)
    );

    // TODO: hardcoded for now
    mohair::SkyResultRel result_rel;
    pushback_plan->relations(0).root()
                               .input()
                               .extension_leaf()
                               .detail()
                               .UnpackTo(&result_rel);

    SkytetherDebugMsg(
         "Result name: " << result_rel.result_name()
      << " (ID: " << std::to_string(result_rel.context_id()) << ")"
    );

    // TODO: for now, going to retrieve from a different process
    // SkytetherTicket query_ticket { result_rel.context_id() };
    // ARROW_RETURN_NOT_OK(RequestResultSet(client_conn, query_ticket));

    std::cout << "Query complete" << std::endl;
    return Status::OK();
  }


  // >> Public entry point

  //! The entry point for the `ClientActions` struct to send all user requests.
  int SendRequests() {
    // Create and connect a FlightClient
    auto client_conn = SkytetherClient::ForLocation(service_loc);
    if (client_conn == nullptr) { return ERRCODE_CONN_CLIENT; }

    // Handle each action depending on what actions were set
    if (query_plan != nullptr) {
      auto status_query = ExecuteQuery(*client_conn);
      if (not status_query.ok()) {
        skytether::PrintError("Unable to execute query plan", status_query);
        return ERRCODE_API_QUERY;
      }
    }

    if (should_shutdown) {
      auto result_shutdown = client_conn->SendSignalShutdown();
      if (not result_shutdown.ok()) {
        skytether::PrintError("Unable to shutdown service", result_shutdown.status());
        return ERRCODE_API_SHUTDOWN;
      }
    }

    if (should_dereg) {
      auto result_dereg = client_conn->SendDeactivation(target_loc);
      if (not result_dereg.ok()) {
        skytether::PrintError("Unable to deregister service", result_dereg.status());
        return ERRCODE_API_DEREGISTER;
      }
    }

    return 0;
  }
};


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "skytether-client"
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
        SkytetherCheckErrCode(errcode_cli, "Failed to parse service location");
        break;
      }

      case 'q': {
        client_actions.query_plan = PlanMessage::FromFile(optarg);
        if (client_actions.query_plan == nullptr) {
          std::cerr << "Failed to parse plan file" << std::endl;
          return ERRCODE_FILE_PARSE;
        }
        break;
      }

      case 'k': {
        client_actions.should_shutdown = true;
        break;
      }

      case 'd': {
        client_actions.should_dereg = true;
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.target_loc));
        SkytetherCheckErrCode(errcode_cli, "Failed to parse target location");
        break;
      }

      default: { break; }
    }
  }

  return client_actions.SendRequests();
}
