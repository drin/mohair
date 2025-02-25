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

// Configuration-based macros
#include "skytether-config.hpp"


// ------------------------------
// Overview
//
// Common macros used throughout this library.


// ------------------------------
// Macros

// macro to print message if debug mode is on
#if SKYTETHER_DEBUG
  #define SkytetherDebugMsg(msg_str)           \
          do {                                 \
            std::cerr << msg_str << std::endl; \
          } while (0);

  #define SkytetherStartTS(phase_name) \
    SteadyTS ts_start_##phase_name = steady_clock::now();

  #define SkytetherStopTS(phase_name) \
    SteadyTS ts_stop_##phase_name = steady_clock::now();

  #define SkytetherLogTimestamps(phase_name) {                                   \
    auto ts_diff = StringifyTSDiff(ts_start_##phase_name, ts_stop_##phase_name); \
    *(LogHandle()) << "["                                                        \
                              << StringifyTS(ts_start_##phase_name) << ":µs"     \
                      << ", " << StringifyTS(ts_stop_##phase_name)  << ":µs"     \
                      << ", " << ts_diff                            << ":µs"     \
                   << "] |> " << #phase_name << std::endl                        \
    ;                                                                            \
  }

  #define SkytetherLogPerf(phase_name, code_block) \
    SkytetherStartTS(phase_name)                   \
    code_block                                  \
    SkytetherStopTS(phase_name)                    \
    SkytetherLogTimestamps(phase_name)

#else
  #define SkytetherDebugMsg(msg_str) {}

#endif

// macro for error code checking boilerplate
#define SkytetherCheckErrCode(err_code, err_msg) \
        do {                                     \
          if (err_code) {                        \
            std::cerr << err_msg << std::endl;   \
            return err_code;                     \
          }                                      \
        } while (0);

