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
#pragma once

// >> Common internal deps
#include "../mohair.hpp"

// >> Internal query processing deps
#include "../query/plans.hpp"

// >> Third-party deps
#include "apidep_flight.hpp"


// ------------------------------
// Functions

// >> Convenience functions
namespace mohair::services {

  // Viewing data structures
  void PrintConfig(ServiceConfig* service_cfg);

  // Internal functions to help initialize a service
  int  SetDefaultLocation(Location *srv_loc);

} // namespace: mohair::services


// >> public API (used from applications using this library)
namespace mohair::services {

  // Functions to start a service
  Status StartService(ServerAdapter& mohair_service, const Location& bind_loc);
  Status StartService(ServerAdapter& mohair_service, const ServiceConfig& service_cfg);

} // namespace: mohair::services


// ------------------------------
// Classes

// >> Service implementations (server processes)

namespace mohair::services {

  //! A custom service that communicates with a CSE via the mohair protocol.
  struct EngineService : public ServerAdapter {

    // >> Class attributes
    static const string hkey_queryticket;

    // >> Instance Attributes
    ServiceConfig service_cfg;

    // >> Deconstructors and Constructors
    virtual ~EngineService() = default;

    EngineService(ShutdownCallback* cb_custom): ServerAdapter(cb_custom) {}
    EngineService()                           : ServerAdapter()          {}

    // >> Convenience functions
    virtual Result<FlightInfo>
    MakeFlightInfo(string partition_key, shared_ptr<Table> data_table);

    virtual Result<FlightInfo>
    MakeFlightInfo(fs::path arrow_fpath, bool is_feather);

    // >> Custom Flight API
    virtual Status DoServiceAction ( const ServerCallContext&  context
                                    ,const Action&             action
                                    ,unique_ptr<ResultStream>* result) override;

    // >> Custom engine API
    virtual Status DoPlanPushdown(
       const ServerCallContext&  context
      ,const shared_ptr<Buffer>  plan_msg
      ,unique_ptr<ResultStream>* result
    );

    virtual Status DoPlanExecution(
       const ServerCallContext&  context
      ,const shared_ptr<Buffer>  plan_msg
      ,unique_ptr<ResultStream>* result
    );

    virtual Status DoViewChange(
       const ServerCallContext&  context
      ,const shared_ptr<Buffer>  service_cfg
      ,unique_ptr<ResultStream>* result
    );

  };

} // namespace: mohair::services
