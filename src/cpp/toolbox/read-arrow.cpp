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

// NOTE: include filesystem before trying to incude <arrow/filesystem/api.h>
#include <filesystem>

#include "../mohair.hpp"

#if USE_DUCKDB
  #include "../engines/adapter_duckdb.hpp"

  using mohair::adapters::EngineDuckDB;
#endif


// ------------------------------
// Type aliases

// >> Namespaces
namespace fs = std::filesystem;



// ------------------------------
// Variables
const std::string local_file_protocol { "file://" };

const char* test_query = (
  "  SELECT  gene_id"
  "         ,COUNT(*)        AS cell_count"
  "         ,AVG(e.expr)     AS expr_avg"
  "         ,VAR_POP(e.expr) AS expr_var"
  "    FROM metaclusters mc"

  "          JOIN clusters c"
  "         USING (cluster_id)"

  "          JOIN cluster_membership cm"
  "         USING (cluster_id)"

  "          JOIN  expression e"
  "         USING (cell_id)"

  "   WHERE mc.mcluster_id = 12"

  "GROUP BY e.gene_id"
);


// ------------------------------
// Functions

int ViewArrowIPCFromFile(fs::path arrow_fpath, bool& is_feather) {
    // Create a RecordBatchStreamReader for the given `arrow_fpath`
    is_feather = true;
    arrow::Result<shared_ptr<Table>> read_result = mohair::ReadIPCFile(arrow_fpath.string());
    if (not read_result.ok()) {
      std::cerr << "Could not read file:"       << std::endl
                << "\t" << read_result.status() << std::endl
      ;

      is_feather = false;
    }

    if (not is_feather) {
      std::cout << "Trying to read file as IPC stream..." << std::endl;

      read_result = mohair::ReadIPCStream(arrow_fpath.string());
      if (not read_result.ok()) {
        std::cerr << "Could not read file:"       << std::endl
                  << "\t" << read_result.status() << std::endl
        ;

        return 2;
      }
    }

    arrow::Result<shared_ptr<Table>> proj_result = read_result;
    if ((*read_result)->num_columns() >= 10) {
      std::cout << "Projecting first 10 columns for readability" << std::endl;

      auto proj_result = (*read_result)->SelectColumns({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
      if (not proj_result.ok()) {
        std::cerr << "Could not do projection:"   << std::endl
                  << "\t" << proj_result.status() << std::endl
        ;
        return 3;
      }
    }

    // print the first 10 rows for readability
    mohair::PrintTable(*proj_result, 0, 10);
    return 0;
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
    if (argc != 2) {
        printf("parse-arrow <path-to-arrow-file>\n");
        return 1;
    }

    fs::path arrow_fpath = fs::absolute(argv[1]).string();

    // Try using DuckDB
    #if USE_DUCKDB
      unique_ptr<EngineDuckDB> duck_engine = mohair::adapters::DuckDBForMem();

      // Use new path, `scan_arrows_file`
      int  context_id     = duck_engine->ArrowScanOpFile(arrow_fpath);
      auto execute_status = duck_engine->ExecuteRelation(context_id);
      if (not execute_status.ok()) { return 4; }
    #endif

    return 0;
}
