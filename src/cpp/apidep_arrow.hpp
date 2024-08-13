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

// >> Arrow core API
#include <arrow/api.h>

// >> Arrow serialization API
#include <arrow/ipc/api.h>

// >> Arrow filesystem API
#include <arrow/filesystem/api.h>


// ------------------------------
// Type aliases

// >> Support types for control flow
using arrow::Result;
using arrow::Status;

// >> Support types for memory management
using arrow::Buffer;

// >> Data types
using arrow::Table;
using arrow::Schema;

// >> Convenience aliases for templated types
using arrow::RecordBatchVector;

// >> Support types for I/O
using arrow::io::RandomAccessFile;
using arrow::ipc::RecordBatchStreamReader;
using arrow::ipc::RecordBatchFileReader;

