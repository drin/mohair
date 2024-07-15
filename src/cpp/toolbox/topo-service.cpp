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
// Type Aliases

using std::unordered_map;

using mohair::ServiceConfig;
using mohair::DeviceClass;


// ------------------------------
// Reference variables
constexpr int argc_min   { 1 };
constexpr int argc_max   { 2 };
constexpr int argndx_loc { 1 };


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
      MohairDebugMsg("Handling request: [register-service]");
      string service_cfg = service_msg->ToString();

      auto service_info = std::make_unique<ServiceConfig>();
      if (not service_info->ParseFromString(service_cfg)) {
        return Status::Invalid("Unable to parse service configuration from message");
      }
      
      ARROW_ASSIGN_OR_RAISE(
         auto service_loc
        ,Location::Parse(service_info->service_location())
      );

      MohairDebugMsg("registering service location: " << service_loc.ToString());
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
  // default status is success
  int errcode_validation { 0 };

  // Error if we have an invalid amount of arguments
  errcode_validation = mohair::services::ValidateArgCount(argc, argc_min, argc_max);
  MohairCheckErrCode(errcode_validation, "Usage: topo-service [<Location URI>]");

  // Error if we have an invalid Uri scheme
  if (argc == 2) {
    errcode_validation = mohair::services::ValidateArgLocationUri(argv[argndx_loc]);
    MohairCheckErrCode(errcode_validation, "Invalid scheme for location URI");
  }

  return errcode_validation;
}


// >> For topological service
int StartServiceTopo(const Location& srv_loc) {
    unique_ptr<FlightServerBase> topo_service = std::make_unique<TopologyService>();
    auto status_service = mohair::services::StartService(topo_service, srv_loc);

    if (not status_service.ok()) {
      mohair::PrintError("Error running topological service", status_service);
      return ERRCODE_START_SRV;
    }

    return 0;
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
  int errcode_validation = ValidateArgs(argc, argv);
  MohairCheckErrCode(errcode_validation, "Failed to validate input command-line args");

  Location service_loc;
  int      errcode_service { 0 };

  // If no location URI is specified, use default location (localhost; any port)
  if (argc == 1) {
    errcode_service = mohair::services::SetDefaultLocation(&service_loc);
    MohairCheckErrCode(errcode_service, "Unable to set default location URI");
  }

  if (argc == 2) {
    errcode_service = mohair::services::ParseArgLocationUri(argv[argndx_loc], &service_loc);
    MohairCheckErrCode(errcode_service, "Unable to parse location URI");
  }

  errcode_service = StartServiceTopo(service_loc);
  MohairCheckErrCode(errcode_service, "Unable to start csd-service");

  return 0;
}
