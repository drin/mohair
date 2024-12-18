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
#include <filesystem>

// >> Internal
#include "skytether.hpp"

//  service-specific includes
#if SKYTETHER_USE_DUCKDB
  #include "../engines/adapter_duckdb.hpp"

  using skytether::adapters::EngineDuckDB;
#endif


// ------------------------------
// Type aliases

// >> Namespaces
namespace fs = std::filesystem;

// >> Types
using skytether::Result;
using skytether::Table;

using std::string;
using std::shared_ptr;

// ------------------------------
// Structs and Classes

struct ToolInterface {
  fs::path arrow_fpath;
  const char* source_name;

  bool is_duckdb_source { false };
  bool is_stream_source { false };
  bool is_stream_output { true  };

  Result<shared_ptr<Table>> ReadDataFromFile() {
    shared_ptr<Table> source_table;

    string arrow_file_uri { "file://" + fs::absolute(source_name).string() };

    if (is_stream_source) {
      ARROW_ASSIGN_OR_RAISE(source_table, skytether::ReadIPCStream(arrow_file_uri));
    }
    else {
      ARROW_ASSIGN_OR_RAISE(source_table, skytether::ReadIPCFile(arrow_file_uri));
    }

    return source_table;
  }

  int WriteStreamFile(shared_ptr<Table> data_table) {
    string arrow_file_uri { "file://" + arrow_fpath.string() };
    auto status_data = skytether::WriteIPCStream(arrow_file_uri, *data_table);
    if (not status_data.ok()) {
      skytether::PrintError("Error writing arrow stream file", status_data);
      return 6;
    }

    return 0;
  }

  int WriteFile(shared_ptr<Table> data_table) {
    string arrow_file_uri { "file://" + arrow_fpath.string() };
    auto status_data = skytether::WriteIPCFile(arrow_file_uri, *data_table);
    if (not status_data.ok()) {
      skytether::PrintError("Error writing arrow file", status_data);
      return 5;
    }

    return 0;
  }

  int Start() {
    if (arrow_fpath.empty()) {
      SkytetherDebugMsg("Missing path to output file.");
      return ERRCODE_CLIENT;
    }

    if (is_duckdb_source) {
      #if SKYTETHER_USE_DUCKDB
        SkytetherDebugMsg("Writing data from DuckDB not yet supported.");
        return 7;

      #else
        SkytetherDebugMsg("DuckDB backend unavailable");
        return 0;

      #endif
    }

    auto result_srctable = ReadDataFromFile();
    if (not result_srctable.ok()) {
      skytether::PrintError("Error reading data from file", result_srctable.status());
      return 8;
    }

    if (is_stream_output) {
      return WriteStreamFile(std::move(result_srctable).ValueOrDie());
    }

    return WriteFile(std::move(result_srctable).ValueOrDie()); 
  }
};


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "read-arrow"
              << " [-h]"
              << " [-d <use duckdb as data source>]"
              << " [-r <read-format (f or s; default is f>]"
              << " [-w <write-format (f or s; default is s)>]"
              << " -i <data-source-name (path or table)>"
              << " -o <path-to-output-file>"
              << "data formats: [fF] is arrow file format, [sS] is arrow stream format"
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
  const     char* opt_template    = "hdr:w:i:o:";

  const char *argv_last = argv[argc - 1];

  char parsed_opt;
  while ((parsed_opt = (char) getopt(argc, argv, opt_template)) != is_done_parsing) {
    switch (parsed_opt) {

      case 'h': { return PrintHelp(); }

      case 'd': {
        my_cli.is_duckdb_source = true;
        break;
      }

      case 'r': {
        if (optarg > argv_last) {
          std::cerr << "Missing argument: <read-format>" << std::endl;
          return PrintHelp();
        }

        switch (optarg[0]) {
          case 's':
          case 'S':
            my_cli.is_stream_source = true;
            break;

          case 'f':
          case 'F':
            my_cli.is_stream_source = false;
            break;

          default:
            std::cerr << "Unknown data format: [" << optarg << "]" << std::endl;
            break;
        }

        break;
      }

      case 'w': {
        if (optarg > argv_last) {
          std::cerr << "Missing argument: <write-format>" << std::endl;
          return PrintHelp();
        }

        switch (optarg[0]) {
          case 's':
          case 'S':
            my_cli.is_stream_output = true;
            break;

          case 'f':
          case 'F':
            my_cli.is_stream_output = false;
            break;

          default:
            std::cerr << "Unknown data format: [" << optarg << "]" << std::endl;
            break;
        }

        break;
      }

      case 'i': {
        my_cli.source_name = optarg;
        break;
      }

      case 'o': {
        my_cli.arrow_fpath = fs::absolute(optarg).string();
        break;
      }

      default: { break; }
    }
  }

  return my_cli.Start();
}
