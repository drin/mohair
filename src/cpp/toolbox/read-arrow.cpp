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

#include "duckdb.hpp"


// >> Aliases
// NOTE: an alias for a namespace must use the `namespace` keyword
namespace fs = std::filesystem;


using duckdb::DuckDB;
using duckdb::Connection;
using duckdb::QueryResult;
using duckdb::DataChunk;
using duckdb::ErrorData;
using duckdb::Value;


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

int ViewQueryResults(duckdb::unique_ptr<QueryResult> result_set) {
  ErrorData             result_err;
  duckdb::unique_ptr<DataChunk> result_chunk { nullptr };

  do {
    if (not result_set->TryFetch(result_chunk, result_err)) {
      std::cerr << result_err.Message() << std::endl;
      return 2;
    }

    if (result_chunk) {
      std::cout << result_chunk->ToString() << std::endl;
    }
  }
  while (result_chunk != nullptr);

  return 0;
}


int UseDuckDB(shared_ptr<Table> data) {
  // "Connect" to database and load extension
  DuckDB testdb;

  // Create a connection through which we can send requests
  Connection duck_conn { testdb };

  // Produce substrait from SQL
  string plan_msg { duck_conn.GetSubstrait(test_query) };

  // Translate substrait to physical plan
  duckdb::vector<Value> fn_args { Value::BLOB_RAW(plan_msg) };
  duckdb::unique_ptr<QueryResult> result = duck_conn.TableFunction("translate_mohair", fn_args)
                                                    ->Execute();

  return ViewQueryResults(std::move(result));
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
    if (argc != 2) {
        printf("parse-arrow <path-to-arrow-file>\n");
        return 1;
    }

    fs::path path_to_arrow = local_file_protocol + fs::absolute(argv[1]).string();

    // Create a RecordBatchStreamReader for the given `path_to_arrow`
    arrow::Result<shared_ptr<Table>> read_result = mohair::ReadIPCFile(path_to_arrow.string());
    if (not read_result.ok()) {
      std::cerr << "Could not read file:"       << std::endl
                << "\t" << read_result.status() << std::endl
      ;
      return 2;
    }

    // project the first 10 columns for readability
    auto result_projection = (*read_result)->SelectColumns({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    if (not result_projection.ok()) {
      std::cerr << "Could not do projection:"         << std::endl
                << "\t" << result_projection.status() << std::endl
      ;
      return 3;
    }

    // print the first 10 rows for readability
    mohair::PrintTable(*result_projection, 0, 10);

    return UseDuckDB(*result_projection);
}
