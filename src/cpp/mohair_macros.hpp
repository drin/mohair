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
#include "mohair-config.hpp"


// ------------------------------
// Overview
//
// Common macros used throughout this library.


// ------------------------------
// Macros

// macro to print message if debug mode is on
#if MOHAIR_DEBUG
  #define MohairDebugMsg(msg_str)              \
          do {                                 \
            std::cerr << msg_str << std::endl; \
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

