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

// >> Configuration-based macros
#include "mohair-config.hpp"

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
#include <arrow/ipc/api.h>
#include <arrow/filesystem/api.h>


// ------------------------------
// Macros

// macro to print message if debug mode is on
#if MOHAIR_DEBUG
  #define MohairDebugMsg(msg_str)              \
          do {                                 \
            std::cout << msg_str << std::endl; \
          } while (0);

#else
  #define MohairDebugMsg(msg_str) {}

#endif

// macro for error code checking boilerplate
#define MohairCheckErrCode(err_code, err_msg)  \
        do {                                   \
          if (err_code) {                      \
            std::cerr << err_msg << std::endl; \
            return err_code;                   \
          }                                    \
        } while (0);


// ------------------------------
// Type aliases


// >> Namespaces
namespace fs = std::filesystem;


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

using arrow::RecordBatchVector;
using arrow::io::RandomAccessFile;
using arrow::ipc::RecordBatchStreamReader;
using arrow::ipc::RecordBatchFileReader;


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
  fstream InputStreamForFile(const char *in_fpath);
  fstream OutputStreamForFile(const char *out_fpath);
  bool    FileToString(const char *in_fpath, string &file_data);

  Result<shared_ptr<Buffer>>
  BufferFromIPCStream(const std::string& path_to_file);

  Result<shared_ptr<Table>>
  ReadIPCFile(const string &path_to_file);

  Result<shared_ptr<Table>>
  ReadIPCStream(const string &path_to_file);

  // >> Convenience Functions
  void PrintTable(shared_ptr<Table> table_data, int64_t offset, int64_t length);
  string JoinStr(vector<string> str_parts, const char *delim);

  // >> Debugging Functions
  void PrintError(const char *msg, const Status arrow_status);

} // namespace: mohair
