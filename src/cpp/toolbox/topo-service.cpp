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

#include "../query/mohair/topology.pb.h"


// ------------------------------
// Macros and Aliases

using std::unordered_map;

using mohair::ServiceConfig;
using mohair::DeviceClass;


// ------------------------------
// Classes

// >> An anonymous namespace for implementations nothing else needs to know about
namespace {

  // ----------
  // Aliases we only need in here
  using mohair::services::MohairService;
  using mohair::services::HashFunctorMohairLocation;


  // ----------
  // Metadata service implementations

  // An ActionType has 2 attributes:
  //  1. type:        std::string
  //  2. description: std::string
  vector<ActionType> SupportedActionsForTopology() {
    return {
       { "register-service"   }, { "Add a service to the CS system"      }
      ,{ "deregister-service" }, { "Remove a service from the CS system" }
    };
  }

  // >> For pure topological information
  using location_map = unordered_map<Location, vector<Location>, HashFunctorMohairLocation>;

  struct TopologyService : public MohairService {

    // |> Attributes
    vector<Location> cs_servers;
    location_map     cs_devices;

    // |> Constructors
    TopologyService() = default;

    // |> Helper functions
    Result<FlightEndpoint> GetDownstreamServices([[maybe_unused]] FlightEndpoint& upstream_srv) {
      // TODO
      // auto downstream_srvs = std::make_unique<FlightEndpoint>();
      return Status::NotImplemented("WIP");
    }

    // >> Custom Flight API
    Status
    ActionRegisterService( [[maybe_unused]] const ServerCallContext&  context
                          ,                 const shared_ptr<Buffer>  service_msg
                          ,[[maybe_unused]] unique_ptr<ResultStream>* response_stream) {
      // NOTE: hoping Buffer implicitly converts to string
      string service_cfg = service_msg->ToString();

      auto service_info = std::make_unique<ServiceConfig>();
      if (not service_info->ParseFromString(service_cfg)) {
        return Status::Invalid("Unable to parse service configuration from message");
      }
      
      ARROW_ASSIGN_OR_RAISE(
         auto service_loc
        ,Location::Parse(service_info->service_location())
      );

      cs_servers.push_back(std::move(service_loc));
      return Status::OK();
    }


    // |> Standard Flight API
    Status ListFlights( [[maybe_unused]] const ServerCallContext&   context
                       ,[[maybe_unused]] const Criteria*            criteria
                       ,[[maybe_unused]] unique_ptr<FlightListing>* listings) override {
      return Status::NotImplemented("WIP");
    }

    Status GetFlightInfo( [[maybe_unused]] const ServerCallContext& context
                         ,[[maybe_unused]] const FlightDescriptor&  request
                         ,[[maybe_unused]] unique_ptr<FlightInfo>*  info) override {
      return Status::NotImplemented("WIP");
    }

    Status PollFlightInfo( [[maybe_unused]] const ServerCallContext& context
                          ,[[maybe_unused]] const FlightDescriptor&  request
                          ,[[maybe_unused]] unique_ptr<PollInfo>*    info) override {
      return Status::NotImplemented("WIP");
    }

    Status GetSchema( [[maybe_unused]] const ServerCallContext  &context
                     ,[[maybe_unused]] const FlightDescriptor   &request
                     ,[[maybe_unused]] unique_ptr<SchemaResult> *schema) override {
      return Status::NotImplemented("WIP");
    }

    Status DoAction( const ServerCallContext&  context
                    ,const Action&             action
                    ,unique_ptr<ResultStream>* result) {

      // known actions
      if (action.type == "register-service") {
        return ActionRegisterService(context, action.body, result);
      }

      /*
      else if (action.type == "deregister-service") {
        return ActionDeregisterService(context, action.body, result);
      }
      */

      // Catch all that returns Status::NotImplemented()
      return ActionUnknown(context, action.type);
    }

    Status ListActions( [[maybe_unused]] const ServerCallContext& context
                       ,                 vector<ActionType>*      actions) override {
      *actions = SupportedActionsForTopology();
      return Status::OK();
    }

  };

} // namespace: anonymous


// ------------------------------
// Functions

// >> Engine-agnostic
int ValidateArgs(int argc, [[maybe_unused]] char **argv) {
  if (argc != 1) {
    std::cerr << "Usage: csd-service" << std::endl;
    return 1;
  }

  return 0;
}


// >> For topological service
int StartServiceTopo() {
    unique_ptr<FlightServerBase> topo_service = std::make_unique<TopologyService>();
    auto status_service = mohair::services::StartService(topo_service);

    if (not status_service.ok()) {
      mohair::PrintError("Error running topological service", status_service);
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

  return StartServiceTopo();
}
