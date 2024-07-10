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

// >> Configuration-based macros
#include "../mohair-config.hpp"

// TODO: re-add faodel if time permits
// #if USE_FAODEL
//   #include "../services/service_faodel.hpp"
// #endif

#if USE_DUCKDB
  #include "../services/service_duckdb.hpp"

  using mohair::adapters::EngineDuckDB;
#endif


// ------------------------------
// Functions

// >> Engine-agnostic
int ValidateArgs(int argc, char **argv) {
  if (argc != 1) {
    std::cerr << "Usage: csd-service" << std::endl;
    return 1;
  }

  return 0;
}


// >> For DuckDB engine
int StartServiceDuckDB() {
    auto status_service = mohair::services::StartMohairDuckDB();

    if (not status_service.ok()) {
      mohair::PrintError("Error running cs-engine [duckdb]", status_service);
      return 2;
    }

    return 0;
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
  int validate_status = ValidateArgs(argc, argv);
  if (validate_status != 0) {
    std::cerr << "Failed to validate input command-line args" << std::endl;
    return validate_status;
  }

  #if USE_DUCKDB
    // Start the flight service
    return StartServiceDuckDB();

  #else
    std::cerr << "No known cs-engine is enabled." << std::endl;
    return 0;

  #endif
}
