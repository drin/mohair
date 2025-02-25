// ------------------------------
// License(s)
//
// >> DuckDB license
// Copyright 2018-2024 Stichting DuckDB Foundation
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be included in all copies
// or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// >> Internal license
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

#include "skytether/engines.hpp"
#include "skytether/engines/duckdb/apidep_duckdb.hpp"


// ------------------------------
// Type aliases

#if SKYTETHER_USE_DUCKDB

  namespace skytether::engines {

    // >> Namespaces
    // An alias to the external duckdb namespace
    namespace duckdb = ::duckdb;


    // >> Templated types
    template <typename... template_params>
    using duck_uptr = duckdb::unique_ptr<template_params...>;

    template <typename... template_params>
    using duck_sptr = duckdb::shared_ptr<template_params...>;


    // >> Simple types

    // Interface types
    using duckdb::DuckDB;
    using duckdb::Connection;

    // Data types
    using DuckDecimal = duckdb::DecimalType;

    using duckdb::LogicalType;
    using duckdb::LogicalTypeId;
    using duckdb::Value;
    using duckdb::BaseStatistics;

    using duckdb::idx_t;                 // uint64_t
    using duckdb::child_list_t;          // template<T> vector<pair<string, T>>
    using duckdb::named_parameter_map_t; // template<T> unordered_map<string, value>

    // Enum types
    using duckdb::LogicalOperatorType;
    using duckdb::OnCreateConflict;

    using duckdb::ExplainType;

    // Query result types
    using duckdb::QueryResult;
    using duckdb::ErrorData;
    using duckdb::DataChunk;
    using duckdb::ResultArrowArrayStreamWrapper;

    // Relation types
    using DuckRel = duckdb::Relation;
    using duckdb::ProjectionRelation;
    using duckdb::FilterRelation;
    using duckdb::LimitRelation;

    using duckdb::AggregateRelation;
    using duckdb::OrderRelation;

    using duckdb::SetOpRelation;
    using duckdb::CrossProductRelation;
    using duckdb::JoinRelation;

    using duckdb::CreateViewRelation;

    // Operator types
    using duckdb::LogicalOperator;

    // Expression types
    using duckdb::ParsedExpression;
    using duckdb::FunctionExpression;
    using duckdb::PositionalReferenceExpression;
    using duckdb::StarExpression;

  } // namespace: skytether::engines

#endif


// ------------------------------
// Functions

#if SKYTETHER_USE_DUCKDB

  namespace skytether::engines {

    //! Prints `col_count` columns of the given chunk starting at `col_offset`
    void
    PrintChunk( DataChunk& src_chunk
               ,idx_t col_offset = 0, idx_t col_count = 15
               ,idx_t row_offset = 0, idx_t row_count = 10);

    //! Prints `chunk_count` chunks of the given result set starting at `chunk_offset`
    Status
    PrintQueryResults( QueryResult& result_set
                      ,idx_t chunk_offset = 0, idx_t chunk_count =  3
                      ,idx_t col_offset   = 0, idx_t col_count   = 15
                      ,idx_t row_offset   = 0, idx_t row_count   = 10);


  } // namespace: skytether::engines::duckdb

#endif


// ------------------------------
// Classes

namespace skytether::engines {

  // >> Public translation functions

  //! Transforms DuckDB Relation to DuckDB Logical Operator
  // unique_ptr<LogicalOperator>  TranspilePlanMessage(DuckRel& plan_rel);

  //! Transforms DuckDB Logical Operator to DuckDB Physical Operator
  // unique_ptr<PhysicalOperator> TranslateLogicalPlan(LogicalOperator& logical_plan, bool optimize);

  // >> DuckDB-specific

  #if SKYTETHER_USE_DUCKDB

    struct TranslatorState {
      Connection*              conn;
      FunctionAnchorMap*       fn_anchors;
      static EngineFunctionMap fn_remaps;

      TranslatorState(Connection* duck_conn, FunctionAnchorMap* anchor_map)
        : conn(duck_conn), fn_anchors(anchor_map) {}
    };

    struct DuckContext : public QueryContext {
      // NOTE: this is a shared ptr because TableFunctions return as a shared ptr
      duck_sptr<DuckRel> duck_plan;

      DuckContext(): QueryContext() {}

      DuckContext(duck_sptr<DuckRel>&& plan_root)
        : QueryContext(), duck_plan(std::move(plan_root)) {}

      DuckContext(duck_sptr<DuckRel>&& plan_root, shared_ptr<Buffer> ctx_data)
        : DuckContext(std::move(plan_root)) {
        this->rel_mem.push_back(std::move(ctx_data));
      }

      static DuckContext* Emplace(ContextMap& ctx_map);
      static DuckContext* Emplace(ContextMap& ctx_map, unique_ptr<DuckContext>&& new_ctx);
    };

    struct EngineDuckDB : public QueryEngine {
      // >> Attributes
      Connection engine_conn;
      string     engine_id;

      // >> Constructors
      EngineDuckDB(DuckDB db, const string& id): engine_conn(db), engine_id(id) {}

      // >> Methods for distributed interaction (cooperative decomposition)

      //! Translates the given `SystemPlan` to a DuckDB internal plan
      duck_uptr<DuckRel> TranslatePlan(SystemPlan& sys_plan);

      unique_ptr<ExtensionLeafRel>
      TranslateResultProjection( ProjectionRelation& result_proj
                                ,size_t              ctx_id
                                ,const string&       srv_loc
                                ,const string&       result_name);

      //! Constructs a simple pushback plan from the given projection operator
      unique_ptr<Plan>
      PushbackForExecPlan( ProjectionRelation& result_proj
                          ,Plan*               src_plan
                          ,size_t              ctx_id
                          ,const string&       srv_loc
                          ,const string&       result_name);

      //! Translates the given `SystemPlan` then returns a Pushback plan
      std::tuple<unique_ptr<Plan>, size_t, string>
      ProcessForExecution(SystemPlan& sys_plan, const string& srv_loc);


      // >> Methods for local interaction

      // These create a QueryContext but return the context ID
      size_t ContextForArrowScanOp(shared_ptr<Buffer> ipc_buffer);
      size_t ContextForArrowScanOp(fs::path arrow_fpath);
      size_t ContextForArrowScanOp(const string& plan_data);

      // These do things with a context ID
      DuckContext* GetDuckContext(size_t ctx_id);

      Status ExecuteContext(size_t context_id) override;
      Status ExecuteContext(size_t context_id, const string& view_name) override;

      Status MaterializeResults(const string& name, RecordBatchVector batches) override;
      Result<shared_ptr<RecordBatchReader>> ScanResults(const string& view_name) override;

      DuckRel* GetRelation(size_t context_id);
    };

    unique_ptr<EngineDuckDB> DuckDBForFile(const string& engine_id, fs::path db_fpath);
    unique_ptr<EngineDuckDB> DuckDBForMem(const string& engine_id);

    //! Construct a DuckDB engine plan from the given `sys_plan`
    duck_uptr<DuckRel>
    FromSystemPlan(EngineDuckDB& engine, SystemPlan& sys_plan);

    //! Entry path to translating an expression
    duck_uptr<ParsedExpression>
    TranslateExpr(TranslatorState& tl_state, const SubstraitExpression& sexpr);

    //! Entry path to translating a data type
    unique_ptr<SubstraitType>
    FromDuckType(const LogicalType& type, bool not_null);

  #endif

} // namespace: skytether::engines

