// ------------------------------
// License
//
// Copyright 2025 Aldrin Montana
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
#include "skytether_cli.hpp"

// >> Topology-specific definitions
#include "services/service_topology.hpp"


// ------------------------------
// Type Aliases

// >> Types
using skytether::services::Location;

using skytether::services::StorageHierarchy;
using skytether::services::SkytetherClient;
using skytether::services::TopologyService;

// >> Functions
using skytether::cli::ParseArgLocationUri;
using skytether::cli::ValidateArgCount;
using skytether::cli::ValidateArgLocationUri;

using skytether::services::StartService;


// ------------------------------
// Reference variables
constexpr int argc_min   { 1 };
constexpr int argc_max   { 2 };
constexpr int argndx_loc { 1 };


// ------------------------------
// Structs and Classes

struct ServiceActions {
  const char* config_fpath;

  ServiceActions(): config_fpath(nullptr) {}

  // Public entry point
  int Start() {
    // Validate args
    if (config_fpath == nullptr) {
      std::cerr << "Topology configuration required" << std::endl;
      return ERRCODE_INV_ARGS;
    }

    // Read the topology config
    auto result_topology = StorageHierarchy::FromFile(config_fpath);
    if (not result_topology.ok()) {
      skytether::PrintError("Failed to parse topology config", result_topology.status());
      return ERRCODE_API_CONFIG;
    }
    auto topology = std::move(result_topology).ValueOrDie();

    // View some stats
    /*
    std::cout << "Topology:" << std::endl
              << "\tService count: "  << topology->labels.size()    << std::endl
              << "\tLocation count: " << topology->locations.size() << std::endl
              << std::endl
    ;
    */

    // View the constructed topology
    topology->PrintTopology();

    /*
    std::cout << "Upstream entries:" << std::endl;
    for (size_t engine_ndx = 0; engine_ndx < topology->labels.size(); ++engine_ndx) {
      Location& engine_loc = topology->locations[engine_ndx];

      // This engine does not have any upstream engines
      if (topology->upstream_links[engine_ndx].empty()) {
        std::cout << "\t" << engine_loc.ToString() << " (root service)" << std::endl;
        continue;
      }

      // This engine has upstream engines
      std::cout << "\t" << engine_loc.ToString() << " (upstream)|> ";
      std::vector<size_t>& upstream_ids = topology->upstream_links[engine_ndx];
      auto                 upstream_itr = upstream_ids.begin();

      std::cout << topology->locations[*upstream_itr].ToString();
      for (; upstream_itr != upstream_ids.end(); ++upstream_itr) {
        std::cout << ", " << topology->locations[*upstream_itr].ToString();
      }
      std::cout << std::endl;
    }
    */

    return 0;
  }
};


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "read-topology"
              << " [-h]"
              << " -f <path-to-config-file>"
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
  const     char* opt_template    = "f:h";

  char parsed_opt;
  while ((parsed_opt = (char) getopt(argc, argv, opt_template)) != is_done_parsing) {
    switch (parsed_opt) {

      case 'h': { return PrintHelp(); }

      case 'f': {
        client_actions.config_fpath = optarg;
        break;
      }

      default: { break; }
    }
  }

  return client_actions.Start();
}

