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

#include "client_mohair.hpp"


// ------------------------------
// Class Implementations

namespace mohair::services {

  // >> Mohair client implementations
  Result<unique_ptr<ResultStream>>
  MohairClient::RegisterService(Location& service_loc) {
    ServiceConfig service_cfg;

    service_cfg.set_allocated_service_location(service_loc.ToString());
    service_cfg.set_platform_class(DeviceClass::DEVICE_CLASS_SERVER);

    string serialized_msg;
    if (not service_cfg.SerializeToString(&serialized_msg)) {
      return Status::Invalid("Unable to serialize ServiceConfig message");
    }

    Action action_register { "register-service", Buffer::FromString(serialized_msg) };
    return conn->DoAction(*conn_opts, action_register);
  }


  static Result<unique_ptr<MohairClient>>
  MohairClient::ForTcpLocation(const string& host, const int port) {
    // Construct an arrow::flight::Location for the service
    ARRROW_ASSIGN_OR_RAISE(auto conn_loc, Location::ForGrpcTcp(host, port));

    // Get a client connected to the service
    ARROW_ASSIGN_OR_RAISE(auto conn_client, FlightClient::Connect(conn_loc));

    return std::make_unique<MohairClient>(std::move(conn_client));
  }

} // namespace: mohair::services

