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

#include "flight_internal.hpp"


// ------------------------------
// Functions

namespace mohair::services {

  // ------------------------------
  // Convenience Methods for Arrow Flight
  Result<Location> default_location() {
    Location default_loc;
    auto status_setloc = arrow::flight::Location::ForGrpcTcp("0.0.0.0", 0, &default_loc);
    if (status_setloc.ok()) { return std::move(default_loc); }

    return status_setloc;
  }

} // namespace: mohair::services
