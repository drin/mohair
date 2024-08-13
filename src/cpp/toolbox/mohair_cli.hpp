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

// >> Standard lib
#include <unistd.h>

// >> Internal dependencies for connecting to mohair services
#include "../services/client_mohair.hpp"


// ------------------------------
// Macros and Type Aliases

// aliases to classes names
using mohair::services::MohairClient;


// ------------------------------
// Functions

// >> Convenience functions

namespace mohair::cli {

  // Support functions
  vector<string> GetUriSchemeWhitelist();

  // CLI arg parsing prototypes
  int ParseArgLocationUri(const char* arg_loc_uri, Location* out_srvloc);
  int ParseArgPlatformClass(const char* arg_pclass, int* out_pclass);

  // CLI arg validation prototypes
  int ValidateArgCount(const int argc, const int argc_exact);
  int ValidateArgCount(const int argc, const int argc_min, const int argc_max);
  int ValidateArgLocationUri(const char* arg_loc_uri);

} // namespace: mohair::toolbox
