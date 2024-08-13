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
#pragma once

// >> Standard libs
#include <unordered_map>

// >> Internal
#include "service_mohair.hpp"


// ------------------------------
// Type aliases

// >> For types in standard lib
using std::unordered_map;


// ------------------------------
// Functions

namespace mohair::services {

  // >> type forwards
  struct ServiceHierarchy;

  // `ActionType` has 2 attributes: <type: std::string>, <descr: std::string>
  vector<ActionType> SupportedActionsForTopology();

  // Constructs a topology from the config
  Result<unique_ptr<ServiceHierarchy>>
  TopologyFromConfig(const char* config_fpath, bool be_verbose = false);

  void PrintTopology(ServiceHierarchy* service_map);

} // namespace: mohair::services


// ------------------------------
// Classes

// >> Support classes
namespace mohair::services {

  // >> Hash functor definitions
  struct HashFunctorMohairTicket {
    std::size_t operator()(const Ticket& mohair_ticket) const;
  };

  struct HashFunctorMohairLocation {
    std::size_t operator()(const Location& mohair_location) const;
  };

  // >> Aliases for templated types
  using service_topology = unordered_map< Location
                                         ,unique_ptr<ServiceConfig>
                                         ,HashFunctorMohairLocation>;

  using upstream_map = unordered_map< Location
                                     ,Location
                                     ,HashFunctorMohairLocation>;

  struct ServiceHierarchy {
    vector<Location> cs_servers;
    service_topology cs_devices;
    upstream_map     upstream_locs;
  };

} // namespace: mohair::services


// >> Service definitions
namespace mohair::services {

  struct TopologyService : public ServerAdapter {

    // Attributes
    unique_ptr<ServiceHierarchy> service_map;

    // Constructors
    TopologyService(unique_ptr<ServiceHierarchy>&& srv_topology)
      : ServerAdapter(), service_map(std::move(srv_topology)) {}

    // Helper functions
    Result<FlightEndpoint>
    GetDownstreamServices(FlightEndpoint& upstream_srv);

    // Custom Flight API
    Status DoServiceAction ( const ServerCallContext&  context
                            ,const Action&             action
                            ,unique_ptr<ResultStream>* result) override;

    // Custom topology API
    virtual Status DoActivateService(
       const ServerCallContext&  context
      ,const shared_ptr<Buffer>  service_msg
      ,unique_ptr<ResultStream>* response_stream
    );

    virtual Status DoDeactivateService(
       const ServerCallContext&  context
      ,const shared_ptr<Buffer>  service_msg
      ,unique_ptr<ResultStream>* response_stream
    );

    // Standard Flight API
    Status ListActions ( const ServerCallContext& context
                        ,vector<ActionType>*      actions) override;

  };

} // namespace: mohair::services
