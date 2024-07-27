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

#include "client_mohair.hpp"


// ------------------------------
// Class Implementations

namespace mohair::services {

  // >> Shutdown callbacks
  Status DeregistrationCallback::operator()() {
    auto result_dereg = client_conn->DeregisterService(*target_loc);
    if (not result_dereg.status().ok()) {
      mohair::PrintError("Unable to de-register service", result_dereg.status());
    }

    MohairDebugMsg("De-registered service");
  }

  // >> Mohair client implementations
  Result<unique_ptr<ResultStream>>
  MohairClient::ShutdownService() {
    Action action_shutdown { "service-shutdown", nullptr };
    return conn->DoAction(conn_opts, action_shutdown);
  }

  Result<unique_ptr<ResultStream>>
  MohairClient::ExecQueryPlan(shared_ptr<Buffer>& plan_msg) {
    Action action_query { "mohair-query", plan_msg };
    return conn->DoAction(conn_opts, action_query);
  }

  Result<unique_ptr<ResultStream>>
  MohairClient::RegisterService(const Location& service_loc) {
    Action action_register { "service-activate", Buffer::FromString(service_loc.ToString()) };
    return conn->DoAction(conn_opts, action_register);
  }

  Result<unique_ptr<ResultStream>>
  MohairClient::DeregisterService(const Location& service_loc) {
    Action action_register { "service-deactivate", Buffer::FromString(service_loc.ToString()) };
    return conn->DoAction(conn_opts, action_register);
  }

  Result<unique_ptr<ResultStream>>
  MohairClient::UpdateView(const ServiceConfig& service_cfg) {
    string serialized_msg;
    if (not service_cfg.SerializeToString(&serialized_msg)) {
      return Status::Invalid("Unable to serialize ServiceConfig message");
    }

    Action action_viewchange { "view-change", Buffer::FromString(serialized_msg) };
    return conn->DoAction(conn_opts, action_viewchange);
  }

} // namespace: mohair::services


// ------------------------------
// Functions

namespace mohair::services {

  Result<unique_ptr<MohairClient>> ClientForLocation(const Location& conn_loc) {
    // Get a client connected to the service
    ARROW_ASSIGN_OR_RAISE(auto conn_client, FlightClient::Connect(conn_loc));

    return std::make_unique<MohairClient>(std::move(conn_client));
  }

} // namespace: mohair::services

