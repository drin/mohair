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
#include "../mohair-config.hpp"
#include "../services/service_mohair.hpp"
#include "../clients/client_mohair.hpp"


// -
// Macros and Type Aliases

// >> Structs
using mohair::services::MohairClient;

// >> Functions
using mohair::services::ClientForLocation;
using mohair::services::ParseArgLocationUri;


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

  // Internal implementations
  Status SendQuery(MohairClient& client_conn) {
    // Send execution request and receive response
    ARROW_ASSIGN_OR_RAISE(auto query_results, client_conn.ExecQueryPlan(request_payload));

    // Iterate over responses (end-of-stream signaled by null result)
    ARROW_ASSIGN_OR_RAISE(auto query_result, query_results->Next());
    while (query_result != nullptr) {
      std::cout << "parsed successful results" << std::endl;
      ARROW_ASSIGN_OR_RAISE(query_result, query_results->Next());
    }

    std::cout << "Query complete" << std::endl;
    return Status::OK();
  }

  // Public entry point
  int SendRequests() {
    // Create and connect a FlightClient
    auto result_client = ClientForLocation(service_loc);
    if (not result_client.ok()) {
      mohair::PrintError("Unable to connect flight client", result_client.status());
      return ERRCODE_CONN_CLIENT;
    }

    // Make the client accessible
    unique_ptr<MohairClient> client_conn = std::move(result_client).ValueOrDie();

    // Handle each action depending on what actions were set
    if (request_payload != nullptr) {
      auto status_query = SendQuery(*client_conn);
      if (not status_query.ok()) {
        mohair::PrintError("Unable to execute query plan", status_query);
        return ERRCODE_API_QUERY;
      }
    }

    if (should_shutdown) {
      auto result_shutdown = client_conn->ShutdownService();
      if (not result_shutdown.status().ok()) {
        mohair::PrintError("Unable to shutdown service", result_shutdown.status());
        return ERRCODE_API_SHUTDOWN;
      }
    }

    if (should_dereg) {
      auto result_dereg = client_conn->DeregisterService(target_loc);
      if (not result_dereg.status().ok()) {
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
        if (not result_buffer.status().ok()) {
          mohair::PrintError("Unable to read plan file", result_buffer.status());
          return ERRCODE_FILE_PARSE;
        }
        client_actions.request_payload = std::move(result_buffer).ValueOrDie();
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
