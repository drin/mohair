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

#include "mohair_cli.hpp"


// ------------------------------
// Functions

// >> General support functions
namespace mohair::cli {

  vector<string> GetUriSchemeWhitelist() {
    static vector<string> whitelist_urischemes;
    whitelist_urischemes.reserve(1);
    whitelist_urischemes.push_back(string { "grpc+tcp://" });

    return whitelist_urischemes;
  }

} // namespace: mohair::cli


// >> CLI arg parsing implementations
namespace mohair::cli {

  int ParseArgLocationUri(const char* arg_loc_uri, Location* out_srvloc) {
    string input_loc { arg_loc_uri };

    auto result_loc = Location::Parse(input_loc);
    if (not result_loc.ok()) {
      std::cerr << "Failed to parse specified location URI:" << std::endl
                << "\t" << result_loc.status().message()     << std::endl
      ;

      return ERRCODE_PARSE_URI;
    }

    *out_srvloc = *result_loc;
    return 0;
  }

  int ParseArgPlatformClass(const char* arg_pclass, int* out_pclass) {
    MohairDebugMsg("Parsing platform class (expecting int32_t)");
    string input_pclass { arg_pclass };

    try {
      *out_pclass = std::stoi(input_pclass);
    }

    catch (std::invalid_argument const& err_arg) {
      std::cerr << "Unable to parse numeric value: " << err_arg.what() << std::endl;
      return ERRCODE_INV_ARGS;
    }

    catch (std::out_of_range const& err_val) {
      std::cerr << "Unexpected value: " << err_val.what() << std::endl;
      return ERRCODE_PARSE_NUMERIC;
    }

    *out_pclass = -1;
    return 0;
  }

} // namespace: mohair::cli


// >> CLI arg validation implementations

namespace mohair::cli {

  int ValidateArgCount(const int argc, const int argc_exact) {
    if (argc != argc_exact) { return ERRCODE_INV_ARGS; }

    return 0;
  }

  int ValidateArgCount(const int argc, const int argc_min, const int argc_max) {
    if (argc < argc_min or argc > argc_max) { return ERRCODE_INV_ARGS; }

    return 0;
  }

  int ValidateArgLocationUri(const char* arg_loc_uri) {
    string loc_uri { arg_loc_uri };

    // comparison against whitelist
    for (const string& uri_scheme : GetUriSchemeWhitelist()) {
      if (loc_uri.compare(0, uri_scheme.length(), uri_scheme) == 0) { return 0; }
    }

    // otherwise, fail
    return ERRCODE_INV_URISCHEME;
  }

} // namespace: mohair::cli
