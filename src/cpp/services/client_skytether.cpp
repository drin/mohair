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

#include "services/client_skytether.hpp"


// ------------------------------
// Functions

namespace skytether::services {
  // >> Reusable function kernels
  Status
  ErrorForEmptyResult(FlightResult* query_result) {
    if (not query_result or not query_result->body) {
      return Status::Invalid("Received empty result.");
    }

    return Status::OK();
  }

  Status
  ErrorForTrailingData(ResultStream& query_results) {
    bool err_trailingdata { false };

    ARROW_ASSIGN_OR_RAISE(unique_ptr<FlightResult> query_result, query_results.Next());
    while (query_result and query_result->body) {
      SkytetherDebugMsg("\tUnexpected trailing results: " << query_result->body->ToString());
      err_trailingdata = true;

      ARROW_ASSIGN_OR_RAISE(query_result, query_results.Next());
    }

    if (err_trailingdata) {
      return Status::Invalid("Received unexpected trailing results");
    }

    return Status::OK();
  }


  // >> Implementations for individual actions

  //! Parse a single SkytetherTicket from `query_results`
  Result<SkytetherTicket>
  ExpectResultFromQuery(unique_ptr<ResultStream> query_results) {
    ARROW_ASSIGN_OR_RAISE(unique_ptr<FlightResult> query_result, query_results->Next());

    ARROW_RETURN_NOT_OK(ErrorForEmptyResult(query_result.get()));
    ARROW_RETURN_NOT_OK(ErrorForTrailingData(*query_results));

    return SkytetherTicket::FromBuffer(query_result->body);
  }

  //! Parse a single Plan from `query_results`, representing the pushback plan
  Result<unique_ptr<Plan>>
  ExpectPushbackFromQuery(unique_ptr<ResultStream> query_results) {
    ARROW_ASSIGN_OR_RAISE(unique_ptr<FlightResult> query_result, query_results->Next());

    ARROW_RETURN_NOT_OK(ErrorForEmptyResult(query_result.get()));
    ARROW_RETURN_NOT_OK(ErrorForTrailingData(*query_results));

    return mohair::SubstraitPlanFromString(query_result->body->ToString());
  }

} // namespace: skytether::services


// ------------------------------
// Class Implementations

namespace skytether::services {

  // >> Method implementations for shutdown callbacks

  //! Callback that sends deactivation request to metadata server
  Status DeactivationCallback::operator()() {
    if (client_conn != nullptr and target_loc != nullptr) {
      SkytetherDebugMsg("Sending deactivation request");
      auto skyconn = static_cast<SkytetherClient*>(client_conn);
      ARROW_RETURN_NOT_OK(skyconn->SendDeactivation(*target_loc));
    }

    return Status::OK();
  }


  // >> Method implementations for SkytetherClient

  // Topology-specific methods
  Result<unique_ptr<ResultStream>>
  SkytetherClient::SendActivation(const Location& service_loc) {
    Action rpc_action { ActionActivate, Buffer::FromString(service_loc.ToString()) };
    return client->DoAction(rpc_opts, rpc_action);
  }

  Result<unique_ptr<ResultStream>>
  SkytetherClient::SendDeactivation(const Location& service_loc) {
    Action rpc_action { ActionDeactivate, Buffer::FromString(service_loc.ToString()) };
    return client->DoAction(rpc_opts, rpc_action);
  }

  Result<unique_ptr<ResultStream>>
  SkytetherClient::SendDecompositionOff(const Location& service_loc) {
    Action rpc_action { ActionDisableDecomp, Buffer::FromString(service_loc.ToString()) };
    return client->DoAction(rpc_opts, rpc_action);
  }

  Result<unique_ptr<ResultStream>>
  SkytetherClient::SendDecompositionOn(const Location& service_loc) {
    Action rpc_action { ActionEnableDecomp, Buffer::FromString(service_loc.ToString()) };
    return client->DoAction(rpc_opts, rpc_action);
  }

  Result<unique_ptr<ResultStream>>
  SkytetherClient::SendViewUpdate(const ServiceConfig& service_cfg) {
    string serialized_msg;
    if (not service_cfg.SerializeToString(&serialized_msg)) {
      return Status::Invalid("Unable to serialize ServiceConfig message");
    }

    Action rpc_action { ActionViewChange, Buffer::FromString(serialized_msg) };
    return client->DoAction(rpc_opts, rpc_action);
  }

  // Engine-specific methods
  Result<unique_ptr<FlightStreamReader>>
  SkytetherClient::GetQueryResults(SkytetherTicket& query_ticket) {
    return client->DoGet(rpc_opts, query_ticket);
  }

  Result<unique_ptr<ResultStream>>
  SkytetherClient::SendPlanMessage(shared_ptr<Buffer> plan_data) {
    Action rpc_action { ActionQuery, plan_data };
    return client->DoAction(rpc_opts, rpc_action);
  }

  unique_ptr<SkytetherClient>
  SkytetherClient::ForLocation(const Location& conn_location) {
    // Get a client connected to the service
    auto result_client = FlightClient::Connect(conn_location);
    if (not result_client.ok()) {
      std::cerr << "Unable to connect to service"   << std::endl
                << result_client.status().message() << std::endl
      ;

      return nullptr;
    }

    // Construct a skytether client that wraps the connected FlightClient
    return std::make_unique<SkytetherClient>(std::move(result_client).ValueOrDie());
  }


  //! Submits a single query plan then validates the response is a `Plan`
  Result<unique_ptr<Plan>>
  SkytetherClient::SendQueryPlan(PlanMessage& plan_msg) {
    ARROW_ASSIGN_OR_RAISE(
       unique_ptr<ResultStream> response
      ,SendPlanMessage(Buffer::FromString(plan_msg.Serialize()))
    );

    return ExpectPushbackFromQuery(std::move(response));
  }

} // namespace: skytether::services

