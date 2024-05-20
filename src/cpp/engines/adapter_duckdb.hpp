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

#include <unordered_map>

//  >> Internal libs
#include "../mohair.hpp"


// ------------------------------
// Type aliases

using std::unordered_map;


// ------------------------------
// Only define if DuckDB is enabled
#if USE_DUCKDB

  // ------------------------------
  // Dependencies

  #include "duckdb.hpp"


  // ------------------------------
  // Type aliases

  // >> Namespaces
  namespace fs = std::filesystem;

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
  using duckdb::DataChunk;
  using duckdb::ErrorData;

  // >> Data types
  using duckdb::idx_t;        // uint64_t
  using duckdb::child_list_t; // template<T> vector<pair<string, T>>

  using duckdb::Value;
  using duckdb::LogicalType;

  // >> Relation types
  using duckdb::Relation;


  // ------------------------------
  // Functions

  namespace mohair::adapters {
    //! Construct a duckdb::Value that is a struct of <ptr, size>
    Value ValueForIPCBuffer(Buffer& ipc_buffer);

    //! Print query results received from DuckDB
    void PrintChunk( DataChunk& src_chunk
                    ,idx_t col_offset = 0, idx_t col_count = 15
                    ,idx_t row_offset = 0, idx_t row_count = 10);

    Status PrintQueryResults( QueryResult& result_set
                             ,idx_t chunk_offset = 0, idx_t chunk_count =  3
                             ,idx_t col_offset   = 0, idx_t col_count   = 15
                             ,idx_t row_offset   = 0, idx_t row_count   = 10);

  } // namespace: mohair::adapters


  // ------------------------------
  // Classes

  namespace mohair::adapters {

    struct QueryContext {
      // >> Attributes
      duck_sptr<Relation>        duck_rel;
      duck_uptr<QueryResult>     rel_result;
      vector<shared_ptr<Buffer>> rel_mem;

      // >> Constructors
      QueryContext() = default;
    };

    struct EngineDuckDB {
      // >> Attributes
      DuckDB     engine_db;
      Connection engine_conn;
      int        context_id;

      // A stash of relations that we need to keep track of
      unordered_map<int, unique_ptr<QueryContext>> query_contexts;

      // >> Constructors
      EngineDuckDB(DuckDB db): engine_conn(db), context_id(0) {}

      // >> Functions
      Result<int> ArrowScanOpIPC(fs::path arrow_fpath);
      Result<int> ArrowScanOpFile(fs::path arrow_fpath);
      Status ExecuteRelation(int prepared_relid);
    };

    EngineDuckDB DuckDBForFile(fs::path db_fpath);
    EngineDuckDB DuckDBForMem();

  } // namespace: mohair::adapters

#endif
