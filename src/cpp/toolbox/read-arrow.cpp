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

// >> Standard libs
#include <unistd.h>
#include <filesystem>

// >> Internal
#include "../mohair.hpp"

//  service-specific includes
#if USE_DUCKDB
  #include "../engines/adapter_duckdb.hpp"

  using mohair::adapters::EngineDuckDB;
#endif


// ------------------------------
// Type aliases

// >> Namespaces
namespace fs = std::filesystem;


// ------------------------------
// Structs and Classes

struct ToolInterface {
  fs::path arrow_fpath;

  int ExecuteFileScan() {
    #if USE_DUCKDB
      unique_ptr<EngineDuckDB> duck_engine = mohair::adapters::DuckDBForMem();

      // Use new path, `scan_arrows_file`
      int  context_id     = duck_engine->ArrowScanOpFile(arrow_fpath);
      auto execute_status = duck_engine->ExecuteRelation(context_id);
      if (not execute_status.ok()) { return 4; }

    #else
      MohairDebugMsg("Unknown service backend");
      return 0;

    #endif
  }

  int Start() {
    if (not arrow_fpath.empty()) { return ExecuteFileScan(); }

    MohairDebugMsg("No data source provided.");
    return ERRCODE_CLIENT;
  }
};


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "read-arrow"
              << " [-h]"
              << " -f <path-to-arrow-IPC-file>"
              << std::endl
    ;

    return 1;
}


// ------------------------------
// Main Logic

int main(int argc, char **argv) {
  ToolInterface my_cli;

  // Parse each argument and internalize the provided option
  constexpr char  is_done_parsing = -1;
  const     char* opt_template    = "f:h";

  char parsed_opt;
  while ((parsed_opt = (char) getopt(argc, argv, opt_template)) != is_done_parsing) {
    switch (parsed_opt) {

      case 'h': { return PrintHelp(); }

      case 'f': {
        my_cli.arrow_fpath = fs::absolute(optarg).string();
        break;
      }

      default: { break; }
    }
  }

  return my_cli.Start();
}
