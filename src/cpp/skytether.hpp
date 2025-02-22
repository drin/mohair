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
#include "skytether-config.hpp" // Configuration-based macros
#include "skytether_macros.hpp" // Common macro definitions
#include "skytether_codes.hpp"  // Common status codes for functions


// >> API dependencies
#include "skytether/apidep_standard.hpp" // standard library
#include "skytether/apidep_arrow.hpp"    // arrow library
#include "skytether/apidep_mohair.hpp"   // mohair library


// ------------------------------
// Macros

#define SkytetherInitLogger(logger_name) {                              \
  *(SkytetherLogger(logger_name)) << "Logger initialized" << std::endl; \
}

#define SkytetherStartTS(phase_name) \
  SteadyTS ts_start_##phase_name = steady_clock::now();

#define SkytetherStopTS(phase_name) \
  SteadyTS ts_stop_##phase_name = steady_clock::now();

#define SkytetherLogTimestamps(phase_name) {                                           \
  auto ts_diff = mohair::StringifyTSDiff(ts_start_##phase_name, ts_stop_##phase_name); \
  *(SkytetherLogger()) << "["                                                          \
                            << mohair::StringifyTS(ts_start_##phase_name) << ":µs"     \
                    << ", " << mohair::StringifyTS(ts_stop_##phase_name)  << ":µs"     \
                    << ", " << ts_diff                                    << ":µs"     \
                 << "] |> " << #phase_name << std::endl                                \
  ;                                                                                    \
}

#define SkytetherLogPerf(phase_name, code_block) \
  SkytetherStartTS(phase_name)                   \
  code_block                                     \
  SkytetherStopTS(phase_name)                    \
  SkytetherLogTimestamps(phase_name)

/* NOTE: for now this needs to always be available
#if SKYTETHER_DEBUG
#else
  #define SkytetherStartTS(phase_name)             {}
  #define SkytetherStopTS(phase_name)              {}
  #define SkytetherLogTimestamps(ts_name, log_msg) {}
  #define SkytetherLogPerf(phase_name, code_block) code_block
#endif
*/


// ------------------------------
// Public global variables

namespace skytether {
  const string version       = SKYTETHER_VERSION_STRING;
  const string version_major = SKYTETHER_VERSION_MAJOR;
  const string version_minor = SKYTETHER_VERSION_MINOR;
  const string version_patch = SKYTETHER_VERSION_PATCH;
};


// ------------------------------
// Public classes and functions

namespace skytether {

  //! A function that returns a singleton file handle for a log file.
  std::fstream* SkytetherLogger(string logger_name);
  std::fstream* SkytetherLogger();

  // >> Reader functions (from files)
  fstream InputStreamForFile(const char* in_fpath);
  fstream OutputStreamForFile(const char* out_fpath);
  bool    FileToString(const char* in_fpath, string& file_data);

  Result<shared_ptr<Buffer>> BufferFromFile(const char* fpath);
  Result<shared_ptr<Buffer>> BufferFromIPCStream(const string& fpath);

  Result<shared_ptr<Table>> ReadIPCFile(const string& fpath);
  Result<shared_ptr<Table>> ReadIPCStream(const string& fpath);

  Status WriteIPCStream(const string& fpath, const Table& data_table);
  Status WriteIPCFile(const string& fpath, const Table& data_table);

  // >> Convenience Functions
  void PrintTable(shared_ptr<Table> table_data, int64_t offset, int64_t length);
  void PrintRecordBatch(shared_ptr<RecordBatch> batch_data, int64_t offset, int64_t length);
  string JoinStr(vector<string> str_parts, const char* delim);

  // >> Debugging Functions
  void PrintError(const char* msg, const Status& arrow_status);

} // namespace: skytether
