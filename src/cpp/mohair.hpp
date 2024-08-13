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
#pragma once


// >> Internal deps

// Configuration-based macros
#include "mohair-config.hpp"

#include "mohair_macros.hpp" // Common macro definitions
#include "mohair_codes.hpp"  // Common status codes for functions

// >> API dependencies

#include "apidep_standard.hpp" // standard library
#include "apidep_arrow.hpp"    // arrow library


// ------------------------------
// Public global variables

namespace mohair {
  const string version       = VERSION_STRING;
  const string version_major = VERSION_MAJOR;
  const string version_minor = VERSION_MINOR;
  const string version_patch = VERSION_PATCH;
};


// ------------------------------
// Public classes and functions

namespace mohair {

  // >> Reader functions (from files)
  fstream InputStreamForFile(const char* in_fpath);
  fstream OutputStreamForFile(const char* out_fpath);
  bool    FileToString(const char* in_fpath, string& file_data);

  Result<shared_ptr<Buffer>> BufferFromFile(const char* fpath);
  Result<shared_ptr<Buffer>> BufferFromIPCStream(const string& fpath);

  Result<shared_ptr<Table>> ReadIPCFile(const string& fpath);
  Result<shared_ptr<Table>> ReadIPCStream(const string& fpath);

  // >> Convenience Functions
  void PrintTable(shared_ptr<Table> table_data, int64_t offset, int64_t length);
  string JoinStr(vector<string> str_parts, const char* delim);

  // >> Debugging Functions
  void PrintError(const char* msg, const Status& arrow_status);

} // namespace: mohair
