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
  using QueryContextMap   = std::unordered_map<size_t, unique_ptr<QueryContext>>;

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

  //! Write batches to an IPC stream
  Result<shared_ptr<Buffer>> SerializeRecordBatches(RecordBatchVector batches);

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

  //! A convenience class wrapping a mapping of UUIDs to query contexts
  struct QueryContext;
  struct ContextMap {
    static size_t next_uuid;

    QueryContextMap contexts;

    //! Adds a `QueryContext` to this instance's QueryContextMap.
    //  This is designed to allow derived classes of QueryContext to be inserted
    QueryContext* RegisterContext(unique_ptr<QueryContext>&& ctx);
    QueryContext* GetContext(size_t context_uuid);
  };

  //! Necessary state to manage execution of a query plan
  struct QueryContext {
    size_t                        uuid;
    atomic<QueryStatus>           status;
    shared_ptr<RecordBatchReader> result;
    vector<shared_ptr<Buffer>>    rel_mem;
    mutex                         status_mtx;
    condition_variable            status_cv;

    virtual ~QueryContext() {}

    QueryContext(): uuid(++ContextMap::next_uuid), status(QueryStatus::Pending) {}
  };

  struct QueryEngine {
    ContextMap context_map;

    virtual ~QueryEngine() {}

    virtual ResultReader ResultSetForContext(size_t context_id);
    virtual Status       ExecuteContext(size_t context_id) = 0;
    virtual Status       ExecuteContext(size_t context_id, const string& view_name) = 0;

    virtual Status CreateView(const string& view_name, RecordBatchVector batches) = 0;

    virtual Result<shared_ptr<RecordBatchReader>>
    ScanResults(const string& view_name) = 0;
  };


  // >> Support classes for bridging betwen system plans and engine plans

  // TODO: need to bridge SystemPlan and EnginePlan

  // TODO: need to bridge MohairOp and EngineOp

  struct QueryOp {
    virtual ~QueryOp() {}
    virtual unique_ptr<SuperPlan> ToSuperPlanRef() { return nullptr; }
  };

} // namespace: skytether::engines

