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

#include "../mohair_macros.hpp"
#include "apidep_flight.hpp"


// ------------------------------
// Adapter Classes

namespace mohair::services {

  // >> Function implementations for ClientAdapter

  // constructors

  // methods
  Result<unique_ptr<ResultStream>>
  ClientAdapter::SendSignalShutdown() {
    Action rpc_action { ActionShutdown, nullptr };
    return client->DoAction(rpc_opts, rpc_action);
  }

  // >> Method implementations for ServerAdapter

  // control flow functions
  Status
  ServerAdapter::DoShutdown(const ServerCallContext& context) {
    MohairDebugMsg("Received shutdown signal from [" << context.peer() << "]");
    if (cb_shutdown != nullptr) { ARROW_RETURN_NOT_OK((*cb_shutdown)()); }

    return Shutdown();
  }

  Status
  ServerAdapter::DoServiceAction( const ServerCallContext&  context
                                 ,const Action&             action
                                 ,unique_ptr<ResultStream>* result) {
    return Status::NotImplemented("Action handler must be implemented by service");
  }


  Status
  ServerAdapter::DoAction( const ServerCallContext&  context
                          ,const Action&             action
                          ,unique_ptr<ResultStream>* result) {
    MohairDebugMsg("Delegating [" << action.type << "] to service");
    return DoServiceAction(context, action, result);
  }

  Status
  ServerAdapter::DoUnknown( const ServerCallContext& context
                           ,const string             action_type) {
    return Status::NotImplemented("Unknown action: [", action_type, "]");
  }

} // namespace: mohair::services
