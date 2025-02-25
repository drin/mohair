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

#include "skytether_macros.hpp"
#include "services/types.hpp"


// ------------------------------
// Adapter Classes

namespace skytether::services {

  // >> Function implementations for SkytetherTicket
  size_t        SkytetherTicket::Id()   { return plan_result->context_id();  }
  const string& SkytetherTicket::Name() { return plan_result->result_name(); }

  shared_ptr<Buffer> SkytetherTicket::ToBuffer() {
    if (tserialized == nullptr) { tserialized = Buffer::FromString(ticket); }
    return tserialized;
  }

  SkytetherTicket SkytetherTicket::FromBuffer(shared_ptr<Buffer> body) {
    string ticket_data = body->ToString();
    auto   sky_result  = std::make_unique<SkyResultRel>();

    sky_result->ParseFromString(ticket_data);
    return SkytetherTicket { ticket_data, std::move(sky_result) };
  }

  SkytetherTicket SkytetherTicket::FromRel(const Rel& result_rel) {
    string ticket_data;
    auto   sky_result = std::make_unique<SkyResultRel>();

    result_rel.extension_leaf().detail().UnpackTo(sky_result.get());
    sky_result->SerializeToString(&ticket_data);

    return SkytetherTicket { ticket_data, std::move(sky_result) };
  }

  SkytetherTicket SkytetherTicket::ForContext(size_t ctx_id, string ctx_name) {
    auto   sky_result = std::make_unique<SkyResultRel>();
    *(sky_result->mutable_result_name()) = ctx_name;
    sky_result->set_context_id(ctx_id);

    string ticket_data;
    sky_result->SerializeToString(&ticket_data);
    return SkytetherTicket { ticket_data, std::move(sky_result) };
  }


  // >> Function implementations for ClientAdapter
  Result<unique_ptr<ResultStream>>
  ClientAdapter::SendSignalShutdown() {
    Action rpc_action { ActionShutdown, nullptr };
    return client->DoAction(rpc_opts, rpc_action);
  }

  Status ClientAdapter::Close() { return client->Close(); }


  // >> Method implementations for ServerAdapter
  Status
  ServerAdapter::DoServiceAction( const ServerCallContext&  context
                                 ,const Action&             action
                                 ,unique_ptr<ResultStream>* result) {
    return Status::NotImplemented("Action handler must be implemented by service");
  }

  Status
  ServerAdapter::DoShutdown(const ServerCallContext& context) {
    SkytetherDebugMsg("Received shutdown signal from [" << context.peer() << "]");
    if (cb_shutdown != nullptr) { ARROW_RETURN_NOT_OK((*cb_shutdown)()); }

    return Shutdown();
  }

  Status
  ServerAdapter::DoUnknown( const ServerCallContext& context
                           ,const string             action_type) {
    return Status::NotImplemented("Unknown action: [", action_type, "]");
  }


  Status
  ServerAdapter::DoAction( const ServerCallContext&  context
                          ,const Action&             action
                          ,unique_ptr<ResultStream>* result) {
    return DoServiceAction(context, action, result);
  }

} // namespace: skytether::services
