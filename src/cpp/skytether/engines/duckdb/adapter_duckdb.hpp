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
    using duckdb::idx_t;                 // uint64_t
    using duckdb::child_list_t;          // template<T> vector<pair<string, T>>
    using duckdb::named_parameter_map_t; // template<T> unordered_map<string, value>
    using duckdb::Value;

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

    // Operator types
    using duckdb::LogicalOperator;

    // Expression types
    using duckdb::ParsedExpression;
    using duckdb::FunctionExpression;
    using duckdb::PositionalReferenceExpression;

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
      duck_sptr<DuckRel> duck_rel;

      DuckContext(duck_sptr<DuckRel>&& rel)
        : QueryContext(), duck_rel(std::move(rel)) {}

      DuckContext(duck_sptr<DuckRel>&& rel, shared_ptr<Buffer> ctx_data)
        : DuckContext(std::move(rel)) {
        this->rel_mem.push_back(std::move(ctx_data));
      }
    };

    struct DuckPlan : public EnginePlan {
      duck_uptr<DuckRel> root_rel;

      DuckPlan(duck_uptr<DuckRel>&& rel): root_rel(std::move(rel)) {}
    };

    struct EngineDuckDB : public QueryEngine {
      // >> Attributes
      Connection engine_conn;

      // >> Constructors
      EngineDuckDB(DuckDB db): engine_conn(db) {}

      // >> Methods
      Status ExecuteFromContext(int32_t context_id) override;

      DuckRel* GetRelation(int32_t context_id);

      int32_t ArrowScanOpIPC(shared_ptr<Buffer> ipc_buffer);
      int32_t ArrowScanOpFile(fs::path arrow_fpath);
      int32_t ExecContextForSubstrait(string plan_msg);

      //! Translates substrait `RelRoot` operator (root operator of a plan) to DuckDB
      duck_uptr<DuckRel> TranslatePlan(SystemPlan& sys_plan);
    };

    unique_ptr<EngineDuckDB> DuckDBForFile(fs::path db_fpath);
    unique_ptr<EngineDuckDB> DuckDBForMem();

    //! Construct a DuckDB engine plan from the given `sys_plan`
    unique_ptr<EnginePlan>
    FromSystemPlan(EngineDuckDB& engine, SystemPlan& sys_plan);

    //! Entry path to translating an expression
    duck_uptr<ParsedExpression>
    TranslateExpr(TranslatorState& tl_state, const SubstraitExpression& sexpr);

  #endif

} // namespace: skytether::engines

