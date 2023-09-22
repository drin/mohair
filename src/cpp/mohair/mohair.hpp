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

// >> Standard libs
#include <functional>
#include <memory>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>

// >> Third-party libs
//  |> Arrow
#include <arrow/api.h>


// ------------------------------
// Type aliases

//  >> Standard types
using std::shared_ptr;
using std::unique_ptr;

using std::string;
using std::stringstream;
using std::fstream;

using std::vector;


//  >> Arrow types
using arrow::Result;
using arrow::Status;

using arrow::Buffer;
using arrow::Table;
using arrow::Schema;


// ------------------------------
// Public classes and functions

namespace mohair {

  //  >> Reader functions (from files)
  fstream StreamForFile(const char *in_fpath);
  bool    FileToString(const char *in_fpath, string &file_data);

  //  >> Convenience Functions
  string JoinStr(vector<string> str_parts, const char *delim);

  //  >> Debugging Functions
  void PrintError(const char *msg, const Status arrow_status);

} // namespace: mohair
