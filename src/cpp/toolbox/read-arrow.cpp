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

#include "../mohair/mohair.hpp"


// >> Aliases
// NOTE: an alias for a namespace must use the `namespace` keyword
namespace fs = std::filesystem;


// ------------------------------
// Variables
const std::string local_file_protocol { "file://" };


// ------------------------------
// Functions


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

    return 0;
}
