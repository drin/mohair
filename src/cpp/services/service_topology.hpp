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

using std::unordered_map;


// ------------------------------
// Functions and Callables

namespace mohair::services {

  // `ActionType` has 2 attributes: <type: std::string>, <descr: std::string>
  vector<ActionType> SupportedActionsForTopology();

  // Constructs a topology from the config
  struct ServiceHierarchy;
  Result<ServiceHierarchy>
  TopologyFromConfig(const char* config_fpath, bool be_verbose = false);

  void PrintTopology(ServiceHierarchy* service_map);

} // namespace: mohair::services


// ------------------------------
// Classes

namespace mohair::services {

  // ----------
  // Hash functor definitions

  struct HashFunctorMohairTicket {
    std::size_t operator()(const Ticket& mohair_ticket) const;
  };

  struct HashFunctorMohairLocation {
    std::size_t operator()(const Location& mohair_location) const;
  };


  // ----------
  // Convenient data structures
  using service_topology = unordered_map<
    Location, unique_ptr<ServiceConfig>, HashFunctorMohairLocation
  >;

  using upstream_map = unordered_map<Location, Location, HashFunctorMohairLocation>;

  struct ServiceHierarchy {
    vector<Location> cs_servers;
    service_topology cs_devices;
    upstream_map     upstream_locs;
  };


  // ----------
  // Service definitions

  struct TopologyService : public MohairService {

    // |> Attributes
    ServiceHierarchy* service_map;

    // |> Helper functions
    Result<FlightEndpoint>
    GetDownstreamServices(FlightEndpoint& upstream_srv);

    // >> Custom Flight API
    Status
    ActionRegisterService( const ServerCallContext&  context
                          ,const shared_ptr<Buffer>  service_msg
                          ,unique_ptr<ResultStream>* response_stream);

    Status
    ActionDeregisterService( const ServerCallContext&  context
                            ,const shared_ptr<Buffer>  service_msg
                            ,unique_ptr<ResultStream>* response_stream);

    // |> Standard Flight API
    Status ListFlights( const ServerCallContext&   context
                       ,const Criteria*            criteria
                       ,unique_ptr<FlightListing>* listings) override;

    Status GetFlightInfo( const ServerCallContext& context
                         ,const FlightDescriptor&  request
                         ,unique_ptr<FlightInfo>*  info) override;

    Status PollFlightInfo( const ServerCallContext& context
                          ,const FlightDescriptor&  request
                          ,unique_ptr<PollInfo>*    info) override;

    Status GetSchema( const ServerCallContext  &context
                     ,const FlightDescriptor   &request
                     ,unique_ptr<SchemaResult> *schema) override;

    Status DoAction( const ServerCallContext&  context
                    ,const Action&             action
                    ,unique_ptr<ResultStream>* result) override;

    Status ListActions( const ServerCallContext& context
                       ,vector<ActionType>*      actions) override;

  };

} // namespace: mohair::services
