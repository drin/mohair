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

// >> Topology-specific definitions
#include "services/service_topology.hpp"


// ------------------------------
// Type Aliases

// classes
using mohair::services::ServiceHierarchy;
using mohair::services::MohairClient;
using mohair::services::TopologyService;

// functions
using mohair::cli::ParseArgLocationUri;
using mohair::cli::ValidateArgCount;
using mohair::cli::ValidateArgLocationUri;

using mohair::services::TopologyFromConfig;
using mohair::services::StartService;


// ------------------------------
// Reference variables
constexpr int argc_min   { 1 };
constexpr int argc_max   { 2 };
constexpr int argndx_loc { 1 };


// ------------------------------
// Functions

// >> Engine-agnostic
int ValidateArgs(int argc, [[maybe_unused]] char **argv) {
  // default status is success
  int errcode_validation { 0 };

  // Error if we have an invalid amount of arguments
  errcode_validation = ValidateArgCount(argc, argc_min, argc_max);
  MohairCheckErrCode(errcode_validation, "Usage: topo-service [<Location URI>]");

  // Error if we have an invalid Uri scheme
  if (argc == 2) {
    errcode_validation = ValidateArgLocationUri(argv[argndx_loc]);
    MohairCheckErrCode(errcode_validation, "Invalid scheme for location URI");
  }

  return errcode_validation;
}


// ------------------------------
// Structs and Classes

struct ServiceActions {
  Location    service_loc;
  const char* config_fpath;
  bool        should_print_topo { false };
  bool        should_verbose    { false };

  ServiceActions(): service_loc(), config_fpath(nullptr) {}

  // Public entry point
  int Start() {
    // Validate args
    if (config_fpath == nullptr) {
      std::cerr << "Topology configuration required" << std::endl;
      return ERRCODE_INV_ARGS;
    }

    // Parse input (topology config)
    auto result_topology = TopologyFromConfig(config_fpath, should_verbose);
    if (not result_topology.ok()) {
      mohair::PrintError("Failed to parse topology config", result_topology.status());
      return ERRCODE_API_CONFIG;
    }
    auto topology = std::move(result_topology).ValueOrDie();

    // Debugging options
    if (should_print_topo) {
      std::cout << std::endl << "Topology:" << std::endl;
      mohair::services::PrintTopology(topology.get());
    }

    if (should_verbose) {
      std::cout << "Upstream entries:" << std::endl;
      for (auto& entry : topology->upstream_locs) {
        std::cout << "\t"   << entry.first.ToString()
                  << " <- " << entry.second.ToString()
                  << std::endl
        ;
      }
    }

    // Create and start the topology service
    auto topo_service = std::make_unique<TopologyService>(std::move(topology));
    auto status_start = StartService(*topo_service, service_loc);
    if (not status_start.ok()) {
      mohair::PrintError("Unable to start topo-service", status_start);
      return ERRCODE_START_SRV;
    }

    return 0;
  }
};


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "topo-service"
              << " [-h]"
              << " -l service-location-uri"
              << " -f path-to-config-file"
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
  const     char* opt_template    = "l:f:hpv";

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

      case 'f': {
        client_actions.config_fpath = optarg;
        break;
      }

      case 'p': {
        client_actions.should_print_topo = true;
        break;
      }

      case 'v': {
        client_actions.should_verbose = true;
        break;
      }

      default: { break; }
    }
  }

  return client_actions.Start();
}
