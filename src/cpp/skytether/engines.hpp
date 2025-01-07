// ------------------------------
// License
//
// Copyright 2024-2025 Aldrin Montana
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

#include "skytether.hpp"


// ------------------------------
// Type aliases

namespace skytether::engines {

  // >> Type forwards
  struct QueryContext;

  // >> Templated types
  using FunctionAnchorMap = std::unordered_map<uint64_t, string>;
  using EngineFunctionMap = std::unordered_map<string  , string>;
  using QueryContextMap   = std::unordered_map<int32_t , unique_ptr<QueryContext>>;

  using ResultReader = shared_ptr<RecordBatchReader>;

} // namespace: skytether::engines


// ------------------------------
// Functions

namespace skytether::engines {

  // >> Support for translation

  //! Remove extension id from a function name
  string FunctionBasename(const string &function_name);

  //! Return the duckdb function name for the given substrait function name
  string RemapFunctionName(const EngineFunctionMap& fn_map, const string& fn_name);

} // namespace: skytether::engines


// ------------------------------
// Classes

namespace skytether::engines {

  // >> Convenience classes

  //! A custom exception for virtual functions with no implementation
  struct NotImplementedError : public std::logic_error {
    explicit NotImplementedError(const char* err_msg = "Not Implemented")
      : std::logic_error(err_msg) {}
  };


  // >> Base classes for engine-specific classes

  //! The state of a query plan's execution
  enum QueryStatus { Pending, Running, Complete };

  //! Necessary state to manage execution of a query plan
  struct QueryContext {
    QueryStatus                   status;
    shared_ptr<RecordBatchReader> result;
    vector<shared_ptr<Buffer>>    rel_mem;

    virtual ~QueryContext() {}

    QueryContext(): status(QueryStatus::Pending) {}
  };

  //! A convenience class wrapping a mapping of UUIDs to query contexts
  struct ContextMap {
    static int32_t next_uuid;

    QueryContextMap contexts;

    int32_t       RegisterContext(unique_ptr<QueryContext>&& new_context);
    QueryContext* GetContext(int32_t context_uuid);
  };

  //! An engine-specific query plan to be optimized and executed.
  struct EnginePlan {
    virtual ~EnginePlan() {}
  };

  struct QueryEngine {
    ContextMap context_map;

    virtual ~QueryEngine() {}

    virtual ResultReader ResultSetForContext(int32_t context_id);
    virtual Status       ExecuteFromContext(int32_t context_id);
  };


  // >> Support classes for bridging betwen system plans and engine plans

  // TODO: need to bridge SystemPlan and EnginePlan

  // TODO: need to bridge MohairOp and EngineOp

  struct QueryOp {
    virtual ~QueryOp() {}
    virtual unique_ptr<SuperPlan> ToSuperPlanRef() { return nullptr;          }
  };

} // namespace: skytether::engines

