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

// >> Internal
#include "mohair_cli.hpp"

// >> Engine-specific definitions
#if USE_DUCKDB
  #include "../services/service_duckdb.hpp"

  using mohair::services::DuckDBService;
#endif


// ------------------------------
// Macros and Type Aliases

// classes
using mohair::services::DeactivationCallback;
using mohair::services::MohairClient;

// functions
using mohair::services::StartService;

using mohair::cli::ParseArgLocationUri;
using mohair::cli::ValidateArgCount;
using mohair::cli::ValidateArgLocationUri;


// ------------------------------
// Structs and Classes

struct ServiceActions {
  Location service_loc;
  Location metasrv_loc;
  bool     backend_isduckdb { true  };
  bool     metasrv_isset    { false };

  unique_ptr<MohairClient>  metasrv_conn { nullptr };
  unique_ptr<ServiceConfig> service_cfg  { nullptr };

  ServiceActions(): service_loc(), metasrv_loc() {}

  // Convenience methods
  int ConnectToMetadataService() {
    metasrv_conn = MohairClient::ForLocation(metasrv_loc);
    if (metasrv_conn == nullptr) {
      std::cerr << "Unable to connect to service" << std::endl;
      return ERRCODE_CONN_CLIENT;
    }

    return 0;
  }

  int ReceiveServiceConfig(unique_ptr<ResultStream> result_stream) {
    // Get the next arrow::flight::Result from the response stream
    // (should only have 1)
    auto result_cfgmsg = result_stream->Next();
    if (not result_cfgmsg.ok()) {
      mohair::PrintError("Failed to get result from stream", result_cfgmsg.status());
      return ERRCODE_API_REGISTER;
    }
    unique_ptr<FlightResult> config_msg = std::move(result_cfgmsg).ValueOrDie();

    // Create a ServiceConfig structure and parse the protobuf message into it
    service_cfg = std::make_unique<ServiceConfig>();
    if (not service_cfg->ParseFromString(config_msg->body->ToString())) {
      std::cerr << "Error parsing initial ServiceConfig" << std::endl;
      return ERRCODE_API_REGISTER;
    }

    // Validate that the config we received is for our location
    if (service_cfg->service_location() != service_loc.ToString()) {
      std::cerr << "Invalid location in configuration. "            << std::endl
                << "\tExpected: " << service_loc.ToString()         << std::endl
                << "\tReceived: " << service_cfg->service_location() << std::endl
      ;
      return ERRCODE_API_REGISTER;
    }

    MohairDebugMsg("Initializing service with config:");
    mohair::services::PrintConfig(service_cfg.get());

    return 0;
  }

  int InitLocalServiceConfig() {
    // Create a ServiceConfig structure and parse the protobuf message into it
    service_cfg = std::make_unique<ServiceConfig>();
    service_cfg->set_service_location(service_loc.ToString());

    MohairDebugMsg("Initializing service with config:");
    mohair::services::PrintConfig(service_cfg.get());

    return 0;
  }

  int RequestActivation() {
    // Activate our place in the topology
    auto result_response = metasrv_conn->SendActivation(service_loc);
    if (not result_response.ok()) {
      mohair::PrintError("Error during service registration", result_response.status());
      return ERRCODE_API_REGISTER;
    }

    return ReceiveServiceConfig(std::move(result_response).ValueOrDie());
  }

  // Public entry point
  int Start() {
    MohairDebugMsg("Starting mohair service [" << service_loc.ToString() << "]");
    int errcode_service { 0 };

    // Prepare a callback; we do it this way to make it optional
    // This is hacky and it's pretty annoying, but fix it later
    DeactivationCallback fn_deactivate;

    // Connect a client to the topology service
    if (metasrv_isset) {
      errcode_service = ConnectToMetadataService();
      MohairCheckErrCode(errcode_service, "Unable to connect to topology service");

      // Make an activation request to the topology service and get our config
      errcode_service = RequestActivation();
      MohairCheckErrCode(errcode_service, "Unable to request activation");

      // Since we connect to a metadata service, point our callback to it
      fn_deactivate.client_conn = metasrv_conn.get();
      fn_deactivate.target_loc  = &service_loc;
    }
    else {
      errcode_service = InitLocalServiceConfig();
    }

    #if USE_DUCKDB
      auto mohair_duckcse = std::make_unique<DuckDBService>(&fn_deactivate);
      auto status_start   = StartService(*mohair_duckcse, *service_cfg);
      if (not status_start.ok()) {
        mohair::PrintError("Unable to start csd-service", status_start);
        return ERRCODE_START_SRV;
      }

      return 0;

    #else
      MohairDebugMsg("No known cs-engine is enabled.");
      return ERRCODE_NO_ENGINE;

    #endif
  }
};


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "csd-service"
              << " [-h]"
              << " -l <service-location>"
              << " -m <topology-service-location>"
              << std::endl
    ;

    return 1;
}


// ------------------------------
// Main Logic

int main(int argc, char **argv) {
  ServiceActions client_actions;

  // Parse each argument and internalize the provided option
  constexpr char  is_done_parsing = -1;
  const     char* opt_template    = "l:m:h";

  char parsed_opt;
  int  errcode_cli;

  while ((parsed_opt = (char) getopt(argc, argv, opt_template)) != is_done_parsing) {
    switch (parsed_opt) {

      case 'h': { return PrintHelp(); }

      case 'l': {
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.service_loc));
        MohairCheckErrCode(errcode_cli, "Failed to parse service location");
        break;
      }

      case 'm': {
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.metasrv_loc));
        MohairCheckErrCode(errcode_cli, "Failed to parse service location");

        client_actions.metasrv_isset = true;
        break;
      }

      default: { break; }
    }
  }

  return client_actions.Start();
}
