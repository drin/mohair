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

// >> Common internal deps
#include "../mohair.hpp"

// >> Internal query processing deps
#include "../query/plans.hpp"

// >> Third-party deps
#include "apidep_flight.hpp"


// ------------------------------
// Classes

namespace mohair::services {

  // >> Client definitions

  //! A client that communicates with a mohair services.
  struct MohairClient: public ClientAdapter {

    // Destructors and Constructors
    virtual ~MohairClient() = default;
    MohairClient(unique_ptr<FlightClient>&& client) : ClientAdapter(std::move(client)) {}

    // Topology-specific Methods
    Result<unique_ptr<ResultStream>> SendViewUpdate(const ServiceConfig& service_cfg);
    Result<unique_ptr<ResultStream>> SendActivation(const Location& service_loc);
    Result<unique_ptr<ResultStream>> SendDeactivation(const Location& service_loc);

    // Engine-specific Methods
    Result<unique_ptr<ResultStream>> SendPlanPushdown(shared_ptr<Buffer>& plan_msg);
    // TODO
    // Result<unique_ptr<ResultStream>> SendPlanResult(shared_ptr<Buffer>& plan_msg);

    // >> Static functions
    static unique_ptr<MohairClient> ForLocation(const Location& conn_location);
  };


  // >> Handlers that implement logic using clients

  //! Handler implementation that deactivates a topology location
  struct DeactivationCallback: public ShutdownCallback {
    ClientAdapter* client_conn;
    Location*      target_loc;

    DeactivationCallback(ClientAdapter* client, Location* loc)
      : client_conn(client), target_loc(loc) {}

    DeactivationCallback(): DeactivationCallback(nullptr, nullptr) {}

    Status operator()() override;
  };

} // namespace: mohair::services
