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

#include "service_mohair.hpp"


// ------------------------------
// Functions

namespace mohair::services {

  Status SetDefaultLocation(Location *srv_loc) {
    // Assign into the location pointed to by `srv_loc`
    ARROW_ASSIGN_OR_RAISE(*srv_loc, Location::ForGrpcTcp("0.0.0.0", 0));

    return Status::OK();
  }

  Status StartService(unique_ptr<FlightServerBase>& service) {
    // Initialize a location
    Location *srv_loc { nullptr };
    ARROW_RETURN_NOT_OK(SetDefaultLocation(srv_loc));

    // Create the service instance
    FlightServerOptions options { *srv_loc };

    std::cout << "Initializing service..." << std::endl;
    ARROW_RETURN_NOT_OK(service->Init(options));

    std::cout << "Setting shutdown signal handler..." << std::endl;
    ARROW_RETURN_NOT_OK(service->SetShutdownOnSignals({SIGTERM}));

    std::cout << "Starting service [localhost:" << service->port() << "]" << std::endl;
    ARROW_RETURN_NOT_OK(service->Serve());

    return Status::OK();
  }

} // namespace: mohair::services
