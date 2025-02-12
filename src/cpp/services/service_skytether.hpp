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
#include "skytether.hpp"
#include "services/types.hpp"
#include "services/client_skytether.hpp"


// ------------------------------
// Functions

// >> Convenience functions
namespace skytether::services {

  // Viewing data structures
  void PrintConfig(ServiceConfig* service_cfg);

  // Internal functions to help initialize a service
  int  SetDefaultLocation(Location *srv_loc);

  //! Submits a single ticket to request query results, then prints the results
  Result<RecordBatchVector>
  RequestResultSet(SkytetherClient& client_conn, SkytetherTicket& query_ticket);

} // namespace: skytether::services


// >> public API (used from applications using this library)
namespace skytether::services {

  // Functions to start a service
  Status StartService(ServerAdapter& skytether_service, const Location& bind_loc);

} // namespace: skytether::services


// ------------------------------
// Classes

// >> Service implementations (server processes)

namespace skytether::services {

  //! A custom service that communicates with a CSE via the skytether protocol.
  struct EngineService : public ServerAdapter {

    // >> Class attributes
    static const string hkey_queryticket;

    // >> Instance Attributes
    unique_ptr<ServiceConfig>           service_cfg;
    vector<unique_ptr<SkytetherClient>> service_conns;

    // >> Constructors and Deconstructors
    EngineService(unique_ptr<ServiceConfig>&& cfg, ShutdownCallback* cb_custom)
      : ServerAdapter(cb_custom), service_cfg(std::move(cfg)) {}

    EngineService(unique_ptr<ServiceConfig>&& cfg)
      : EngineService(std::move(cfg), nullptr) {}

    virtual ~EngineService() = default;


    // >> Convenience functions
    virtual Status ConnectToTopology();

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

} // namespace: skytether::services
