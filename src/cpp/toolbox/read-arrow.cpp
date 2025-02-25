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
#include "skytether.hpp"

//  service-specific includes
#if SKYTETHER_USE_DUCKDB
  #include "skytether/engines.hpp"
  #include "skytether/engines/duckdb/apidep_duckdb.hpp"
  #include "skytether/engines/duckdb/adapter_duckdb.hpp"

  using skytether::engines::EngineDuckDB;
#endif


// ------------------------------
// Type aliases

// >> Namespaces
namespace fs = std::filesystem;

using std::vector;
using std::unique_ptr;


// ------------------------------
// Structs and Classes

struct ToolInterface {
  fs::path arrow_fpath;
  bool use_duckdb { false };
  bool is_stream  { false };
  int  col_limit  { 5     };

  #if SKYTETHER_USE_DUCKDB
    int ScanFileWithDuckDB() {
      unique_ptr<EngineDuckDB> duck_engine = skytether::engines::DuckDBForMem("local");

      // Use new path, `scan_arrows_file`
      int  context_id     = duck_engine->ContextForArrowScanOp(arrow_fpath);
      auto execute_status = duck_engine->ExecuteContext(context_id);
      if (not execute_status.ok()) { return 4; }
    }
  #endif

  vector<int> IndicesForSelection(int table_colcount) {
    int sel_size = col_limit;
    if (table_colcount < sel_size) { sel_size = table_colcount; }

    vector<int> col_selection;
    col_selection.reserve(sel_size);
    for (int col_ndx = 0; col_ndx < sel_size; ++col_ndx) {
      col_selection.push_back(col_ndx);
    }

    return col_selection;
  }

  int ScanStreamFromFile() {
    std::string arrow_file_uri { "file://" + arrow_fpath.string() };
    auto result_data = skytether::ReadIPCStream(arrow_file_uri);
    if (not result_data.ok()) {
      skytether::PrintError("Error reading data stream from file", result_data.status());
      return 6;
    }

    vector<int> col_selection = IndicesForSelection((*result_data)->num_columns());
    auto data_excerpt = (*result_data)->SelectColumns(col_selection);
    if (not data_excerpt.ok()) {
      skytether::PrintError("Error projecting table columns", data_excerpt.status());
      return 9;
    }

    skytether::PrintTable(*data_excerpt, 0, 10);
    return 0;
  }

  int ScanFile() {
    std::string arrow_file_uri { "file://" + arrow_fpath.string() };
    auto result_data = skytether::ReadIPCFile(arrow_file_uri);
    if (not result_data.ok()) {
      skytether::PrintError("Error reading data from file", result_data.status());
      return 5;
    }

    vector<int> col_selection = IndicesForSelection((*result_data)->num_columns());
    auto data_excerpt = (*result_data)->SelectColumns(col_selection);
    if (not data_excerpt.ok()) {
      skytether::PrintError("Error projecting table columns", data_excerpt.status());
      return 9;
    }

    skytether::PrintTable(*data_excerpt, 0, 10);
    return 0;
  }

  int Start() {
    if (arrow_fpath.empty()) {
      SkytetherDebugMsg("No data source provided.");
      return ERRCODE_CLIENT;
    }

    if (use_duckdb) {
      #if SKYTETHER_USE_DUCKDB
        return ScanFileWithDuckDB();

      #else
        SkytetherDebugMsg("DuckDB backend unavailable");
        return 0;

      #endif
    }

    if (is_stream)  { return ScanStreamFromFile(); }

    return ScanFile(); 
  }
};


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "read-arrow"
              << " [-h]"
              << " [-f <read file as arrow file (default)>]"
              << " [-s <read file as arrow stream>]"
              << " [-d <read file through duckdb>]"
              << " -p <path-to-file>"
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
  const     char* opt_template    = "hdsfp:";

  char parsed_opt;
  while ((parsed_opt = (char) getopt(argc, argv, opt_template)) != is_done_parsing) {
    switch (parsed_opt) {

      case 'h': { return PrintHelp(); }

      case 's': {
        my_cli.is_stream = true;
        break;
      }

      case 'd': {
        my_cli.use_duckdb = true;
        break;
      }

      case 'p': {
        my_cli.arrow_fpath = fs::absolute(optarg).string();
        break;
      }

      default: { break; }
    }
  }

  return my_cli.Start();
}
