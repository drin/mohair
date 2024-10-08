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

#include "services/client_mohair.hpp"


// ------------------------------
// Functions

namespace mohair::services {

  // >> Implementation for shutdown callback that sends deactivation request
  Status DeactivationCallback::operator()() {
    if (client_conn != nullptr and target_loc != nullptr) {
      MohairDebugMsg("Sending deactivation request");
      auto mohair_conn = static_cast<MohairClient*>(client_conn);
      ARROW_RETURN_NOT_OK(mohair_conn->SendDeactivation(*target_loc));
    }

    return Status::OK();
  }

} // namespace: mohair::services


// ------------------------------
// Class Implementations

namespace mohair::services {

  // >> Method implementations for MohairClient

  // Topology-specific methods
  Result<unique_ptr<ResultStream>>
  MohairClient::SendActivation(const Location& service_loc) {
    Action rpc_action { ActionActivate, Buffer::FromString(service_loc.ToString()) };
    return client->DoAction(rpc_opts, rpc_action);
  }

  Result<unique_ptr<ResultStream>>
  MohairClient::SendDeactivation(const Location& service_loc) {
    Action rpc_action { ActionDeactivate, Buffer::FromString(service_loc.ToString()) };
    return client->DoAction(rpc_opts, rpc_action);
  }

  Result<unique_ptr<ResultStream>>
  MohairClient::SendViewUpdate(const ServiceConfig& service_cfg) {
    string serialized_msg;
    if (not service_cfg.SerializeToString(&serialized_msg)) {
      return Status::Invalid("Unable to serialize ServiceConfig message");
    }

    Action rpc_action { ActionViewChange, Buffer::FromString(serialized_msg) };
    return client->DoAction(rpc_opts, rpc_action);
  }

  // Engine-specific methods
  Result<unique_ptr<ResultStream>>
  MohairClient::SendPlanPushdown(shared_ptr<Buffer>& plan_msg) {
    Action rpc_action { ActionQuery, plan_msg };
    return client->DoAction(rpc_opts, rpc_action);
  }

  /* TODO
  Result<unique_ptr<ResultStream>>
  MohairClient::SendPlanResult(shared_ptr<Buffer>& plan_msg) {
    Action rpc_action { ActionQuery, plan_msg };
    return client->DoAction(rpc_opts, rpc_action);
  }
  */

  unique_ptr<MohairClient>
  MohairClient::ForLocation(const Location& conn_location) {
    // Get a client connected to the service
    auto result_client = FlightClient::Connect(conn_location);
    if (not result_client.ok()) {
      std::cerr << "Unable to connect to service"   << std::endl
                << result_client.status().message() << std::endl
      ;

      return nullptr;
    }

    // Construct a mohair client that wraps the connected FlightClient
    return std::make_unique<MohairClient>(std::move(result_client).ValueOrDie());
  }

} // namespace: mohair::services

