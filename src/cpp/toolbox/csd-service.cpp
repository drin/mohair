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

// >> Client dependencies
#include "../clients/client_mohair.hpp"

// >> Service dependencies
// TODO: re-add faodel if time permits
// #if USE_FAODEL
//   #include "../services/service_faodel.hpp"
// #endif

#if USE_DUCKDB
  #include "../services/service_duckdb.hpp"

  using mohair::adapters::EngineDuckDB;
#endif


// ------------------------------
// Macros and Type Aliases

// classes
using mohair::services::MohairClient;

// functions
using mohair::services::ValidateArgCount;
using mohair::services::ValidateArgLocationUri;
using mohair::services::ParseArgLocationUri;

using mohair::services::ClientForLocation;


// ------------------------------
// Reference variables
constexpr int argc_min       { 2 };
constexpr int argc_max       { 3 };
constexpr int argndx_metaloc { 1 };
constexpr int argndx_myloc   { 2 };


// ------------------------------
// Functions

// >> Engine-agnostic
int ValidateArgs(int argc, char **argv) {
  // default status is success
  int errcode_validation { 0 };

  // Error if we have an invalid amount of arguments
  errcode_validation = ValidateArgCount(argc, argc_min, argc_max);
  MohairCheckErrCode(errcode_validation, "Usage: csd-service [<Location URI>]");

  // Error if we have an invalid Uri scheme for metadata service
  errcode_validation = ValidateArgLocationUri(argv[argndx_metaloc]);
  MohairCheckErrCode(errcode_validation, "Invalid scheme for location URI (metadata service)");

  // Error if we have an invalid Uri scheme for this service
  if (argc == 3) {
    errcode_validation = ValidateArgLocationUri(argv[argndx_myloc]);
    MohairCheckErrCode(errcode_validation, "Invalid scheme for location URI (this service)");
  }

  return errcode_validation;
}


// >> For DuckDB engine
int StartServiceDuckDB(int argc, char** argv) {
  MohairDebugMsg("Starting mohair service [duckdb]");
  int errcode_service { 0 };

  // Initialize our location
  Location service_loc;

  // If no location URI is specified, use default location (localhost; any port)
  if (argc == 2) {
    errcode_service = mohair::services::SetDefaultLocation(&service_loc);
    MohairCheckErrCode(errcode_service, "Unable to set default location URI");
  }

  // Otherwise, use specified location
  else if (argc == 3) {
    errcode_service = ParseArgLocationUri(argv[argndx_myloc], &service_loc);
    MohairCheckErrCode(errcode_service, "Unable to parse location URI for this service");
  }

  // Grab location of metadata service and register our location
  Location meta_loc;
  errcode_service = ParseArgLocationUri(argv[argndx_metaloc], &meta_loc);
  MohairCheckErrCode(errcode_service, "Unable to parse location URI for metadata service");

  auto result_client = ClientForLocation(meta_loc);
  if (not result_client.ok()) {
    mohair::PrintError("Unable to connect flight client", result_client.status());
    return ERRCODE_CONN_CLIENT;
  }

  unique_ptr<MohairClient> meta_conn = std::move(result_client).ValueOrDie();
  auto result_register = meta_conn->RegisterService(service_loc);
  if (not result_register.ok()) {
    mohair::PrintError("Error during service registration", result_register.status());
    return ERRCODE_API_REGISTER;
  }

  // Run this service until it dies
  errcode_service = mohair::services::StartMohairDuckDB(service_loc);
  MohairCheckErrCode(errcode_service, "Unable to start csd-service");


  // Now that we're done, de-register our location

  return 0;
}


// ------------------------------
// Main Logic
int main(int argc, char** argv) {
  int errcode_validation = ValidateArgs(argc, argv);
  MohairCheckErrCode(errcode_validation, "Failed to validate input command-line args");

  #if USE_DUCKDB
    return StartServiceDuckDB(argc, argv);

  #else
    std::cerr << "No known cs-engine is enabled." << std::endl;
    return ERRCODE_NO_ENGINE;

  #endif
}
