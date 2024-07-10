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

// >> Standard lib
#include <unordered_map>

// >> Internal
#include "../services/service_mohair.hpp"


// ------------------------------
// Macros and Aliases

using std::unordered_map;


// ------------------------------
// Classes

// >> A class directly defined here because nothing else needs to know about it
struct TopologyService : public MohairService {

  // >> Attributes
  unordered_map<FlightEndpoint, vector<Location>> service_map;


  // >> Constructors
  TopologyService() = default;


  // >> Helper functions
  Result<FlightEndpoint> GetDownstreamServices(FlightEndpoint& upstream_srv) {
    // TODO
    // auto downstream_srvs = std::make_unique<FlightEndpoint>();
    return Status::NotImplemented("WIP");
  }


  // >> Standard Flight API
  Status ListFlights( const ServerCallContext&   context
                     ,const Criteria*            criteria
                     ,unique_ptr<FlightListing>* listings) override {
  }

  Status GetFlightInfo( const ServerCallContext& context
                       ,const FlightDescriptor&  request
                       ,unique_ptr<FlightInfo>*  info) override {
  }

  Status PollFlightInfo( const ServerCallContext& context
                        ,const FlightDescriptor&  request
                        ,unique_ptr<PollInfo>*    info) override {
  }

  Status GetSchema( const ServerCallContext  &context
                   ,const FlightDescriptor   &request
                   ,unique_ptr<SchemaResult> *schema) override {
  }

  Status ListActions( const ServerCallContext& context
                     ,vector<ActionType>*      actions) override {
  }

};


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
