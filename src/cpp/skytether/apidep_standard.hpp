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


// ------------------------------
// Overview
//
// Dependencies from standard library that are common throughout this library.


// ------------------------------
// Dependencies

// >> Memory and data type support
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>

// >> Function support
#include <functional>

// >> Data structures
#include <vector>

// >> I/O support
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

// >> Concurrency
#include <atomic>
#include <mutex>
#include <condition_variable>

// >> Performance and timing support
#include <chrono>


// ------------------------------
// Type aliases

namespace skytether {
  // >> namespace aliases
  namespace fs = std::filesystem;

  // >> pointer type aliases
  using std::shared_ptr;
  using std::unique_ptr;

  // >> data type aliases
  using std::string;
  using std::optional;

  // >> data structure type aliases
  using std::vector;
  using std::unordered_map;

  // >> I/O type aliases
  using std::stringstream;
  using std::fstream;

  // >> Safety type aliases
  using std::atomic;
  using std::mutex;
  using std::condition_variable;
  using std::unique_lock;
  using std::lock_guard;

} // namespace: skytether

