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

//  >> Common internal libs
#include "skytether.hpp"


#if SKYTETHER_USE_DUCKDB

  #include "duckdb.hpp"
  #include "duckdb/common/arrow/result_arrow_wrapper.hpp"

  #include <unordered_map>

#endif


// ------------------------------
// Type aliases

namespace skytether {

  // >> standard types
  using std::unordered_map;

  // >> engine types
  #if SKYTETHER_USE_DUCKDB

    // >> Low-level types
    template <typename ptype>
    using duck_sptr = duckdb::shared_ptr<ptype>;

    template <typename ptype>
    using duck_uptr = duckdb::unique_ptr<ptype>;

    // >> Common types
    using duckdb::DuckDB;
    using duckdb::Connection;

    // >> Query result types
    using duckdb::QueryResult;
    using duckdb::ErrorData;
    using duckdb::DataChunk;
    using duckdb::ResultArrowArrayStreamWrapper;

    // >> Data types
    using duckdb::idx_t;        // uint64_t
    using duckdb::child_list_t; // template<T> vector<pair<string, T>>

    using duckdb::Value;
    using duckdb::LogicalType;

    // >> Relation types
    using duckdb::Relation;

  #endif

} // namespace: skytether


// ------------------------------
// Functions

namespace skytether::adapters {

  #if SKYTETHER_USE_DUCKDB

    //! Construct a duckdb::Value that is a struct of <ptr, size>
    Value ValueForIPCBuffer(Buffer& ipc_buffer);

    //! Construct an arrow::RecordBatchReader that wraps the QueryResult
    Result<shared_ptr<RecordBatchReader>>
    ReaderForResult(unique_ptr<QueryResult> result_set);

    //! Print query results received from DuckDB
    void
    PrintChunk( DataChunk& src_chunk
               ,idx_t col_offset = 0, idx_t col_count = 15
               ,idx_t row_offset = 0, idx_t row_count = 10);

    Status
    PrintQueryResults( QueryResult& result_set
                      ,idx_t chunk_offset = 0, idx_t chunk_count =  3
                      ,idx_t col_offset   = 0, idx_t col_count   = 15
                      ,idx_t row_offset   = 0, idx_t row_count   = 10);

  #endif

} // namespace: skytether::adapters


// ------------------------------
// Classes

namespace skytether::adapters {

  enum QueryStatus {
     Pending
    ,Running
    ,Complete
  };

  struct QueryContext {
    QueryStatus                   status { QueryStatus::Pending };
    shared_ptr<RecordBatchReader> rel_result;
    vector<shared_ptr<Buffer>>    rel_mem;
  };

  // >> DuckDB-specific

  #if SKYTETHER_USE_DUCKDB

    struct DuckContext : public QueryContext {
      duck_sptr<Relation> duck_rel;
    };

    struct EngineDuckDB {
      // >> Attributes
      Connection       engine_conn;
      int              context_id;

      // A stash of relations that we need to keep track of
      unordered_map<int, unique_ptr<DuckContext>> query_contexts;

      // >> Constructors
      EngineDuckDB(DuckDB db): engine_conn(db), context_id(0) {}

      // >> Functions
      int ArrowScanOpIPC(shared_ptr<Buffer> ipc_buffer);
      int ArrowScanOpFile(fs::path arrow_fpath);
      int ExecContextForSubstrait(string plan_msg);

      Status                        ExecuteRelation(int context_id);
      Relation&                     GetRelation(int context_id);
      shared_ptr<RecordBatchReader> GetResultSet(int context_id);
    };

    unique_ptr<EngineDuckDB> DuckDBForFile(fs::path db_fpath);
    unique_ptr<EngineDuckDB> DuckDBForMem();

  #endif

} // namespace: skytether::adapters

