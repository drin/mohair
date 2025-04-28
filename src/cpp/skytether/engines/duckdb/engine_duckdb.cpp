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

#include "skytether/engines/duckdb/adapter_duckdb.hpp"


// ------------------------------
// Functions

#if SKYTETHER_USE_DUCKDB

  namespace skytether::engines {
    
    //! Instantiate a wrapper around an in-memory DuckDB database
    unique_ptr<EngineDuckDB> DuckDBForMem(const string& engine_id) {
      DuckDB mem_db;
      return std::make_unique<EngineDuckDB>(mem_db, engine_id);
    }

    //! Instantiate a wrapper around a file-backed DuckDB database
    unique_ptr<EngineDuckDB> DuckDBForFile(const string& engine_id, fs::path db_fpath) {
      DuckDB disk_db(db_fpath);
      return std::make_unique<EngineDuckDB>(disk_db, engine_id);
    }

    //! Entry point into translating a `SystemPlan` into an `EnginePlan` for DuckDB
    duck_uptr<DuckRel>
    FromSystemPlan(EngineDuckDB& engine, SystemPlan& sys_plan) {
      return engine.TranslatePlan(sys_plan);
    }

    //! Prints `col_count` columns of the given chunk starting at `col_offset`
    void PrintChunk( DataChunk& src_chunk
                    ,idx_t col_offset, idx_t col_count
                    ,idx_t row_offset, idx_t row_count) {
      std::cout << "Chunk - [" << std::to_string(col_count) << " Columns]"
                << std::endl
      ;

      idx_t col_ndx { col_offset };
      for (; col_ndx < src_chunk.ColumnCount() && col_ndx < col_count; ++col_ndx) {
        idx_t view_length { row_count - row_offset };
        if (view_length > src_chunk.size()) { view_length = src_chunk.size(); }

        duckdb::Vector col_view {
          src_chunk.data[col_ndx],  row_offset, row_count
        };

        std::cout << "- " << col_view.ToString(view_length) << std::endl;
      }
    }

    Status PrintQueryResults( QueryResult& result_set
                             ,idx_t chunk_offset, idx_t chunk_count
                             ,idx_t col_offset  , idx_t col_count
                             ,idx_t row_offset  , idx_t row_count) {
      idx_t     chunk_ndx { 0 };
      ErrorData result_err;

      // Grab first chunk
      duck_uptr<DataChunk> result_chunk { nullptr };
      auto fetch_result = result_set.TryFetch(result_chunk, result_err);

      // Iterate over chunks
      while (fetch_result && result_chunk != nullptr) {
        if      (chunk_ndx <                  chunk_offset) { continue; }
        else if (chunk_ndx >= (chunk_offset + chunk_count)) { break;    }

        // Print current chunk
        PrintChunk(*result_chunk, col_offset, col_count, row_offset, row_count);

        // Grab next chunk
        fetch_result = result_set.TryFetch(result_chunk, result_err);
        ++chunk_ndx;
      }

      if (not fetch_result) {
        std::cerr << result_err.Message() << std::endl;
        return Status::Invalid(
          "DuckDB: Failed to fetch result chunk " + std::to_string(chunk_ndx)
        );
      }

      return Status::OK();
    }

    //! Construct a duckdb::Value for an arrow IPC buffer (struct of <ptr, size>)
    Value ValueForIPCBuffer(Buffer& ipc_buffer) {
      // Place values into a child_list_t<type>
      child_list_t<Value> struct_vals {
         { "ptr" , Value::UBIGINT((uintptr_t) ipc_buffer.mutable_data()) }
        ,{ "size", Value::UBIGINT((uint64_t)  ipc_buffer.size())         }
      };

      // Call the STRUCT builder function
      return Value::STRUCT(struct_vals);
    }

    //! Construct RecordBatchReader wrapping a QueryResult (via the C-data interface)
    //  ArrowArrayStream keeps a reference to its wrapper and gets properly cleaned up
    //  when the imported RecordBatchReader gets deconstructed.
    Result<shared_ptr<RecordBatchReader>>
    ReaderForResult(duck_uptr<QueryResult> result_set) {
      constexpr size_t result_batchsize { 2048 }; // TODO: parameterize

      SkytetherDebugMsg("Creating Arrow reader for duckdb results");
      auto stream_wrapper = new ResultArrowArrayStreamWrapper(
        std::move(result_set), result_batchsize
      );

      return arrow::ImportRecordBatchReader(&(stream_wrapper->stream));
    }

    //! Convenience function to get a DuckContext from `ctx_map`
    DuckContext* EngineDuckDB::GetDuckContext(size_t ctx_id) {
      return dynamic_cast<DuckContext*>(context_map.GetContext(ctx_id));
    }

  } // namespace: skytether::engines

#endif


// ------------------------------
// Class Implementations


#if SKYTETHER_USE_DUCKDB

  namespace skytether::engines {


    // >> TranslatorState
    //! Static mapping from substrait function names to duckdb function names
    EngineFunctionMap TranslatorState::fn_remaps {
         {"modulus"    , "mod"      }
        ,{"std_dev"    , "stddev"   }
        ,{"starts_with", "prefix"   }
        ,{"ends_with"  , "suffix"   }
        ,{"substring"  , "substr"   }
        ,{"char_length", "length"   }
        ,{"is_nan"     , "isnan"    }
        ,{"is_finite"  , "isfinite" }
        ,{"is_infinite", "isinf"    }
        ,{"like"       , "~~"       }
        ,{"extract"    , "date_part"}
      };

    // >> DuckContext
    DuckContext*
    DuckContext::Emplace(ContextMap& ctx_map) {
      auto new_ctx = std::make_unique<DuckContext>();
      return static_cast<DuckContext*>(ctx_map.RegisterContext(std::move(new_ctx)));
    }

    DuckContext*
    DuckContext::Emplace(ContextMap& ctx_map, unique_ptr<DuckContext>&& new_ctx) {
      return static_cast<DuckContext*>(ctx_map.RegisterContext(std::move(new_ctx)));
    }

    // >> EngineDuckDB
    //! Calculate an estimated memory size for DuckDB
    size_t EngineDuckDB::EstimateTotalTableSize() {
      size_t count_bytes { 0 };

      auto& engine_buffermgr = BufferManager::GetBufferManager(*engine_conn.context);
      for (const MemoryInformation& mem_info : engine_buffermgr.GetMemoryUsageInfo()) {
        switch (mem_info.tag) {
          case MemoryTag::BASE_TABLE:
          case MemoryTag::IN_MEMORY_TABLE:
            count_bytes += mem_info.size;

          default: continue;
        }
      }

      return count_bytes;
    }

    //! Create a duckdb scan operator from an IPC buffer (extracted from an arrow file)
    size_t EngineDuckDB::ContextForArrowScanOp(shared_ptr<Buffer> ipc_buffer) {
      // `scan_arrow_ipc` takes IPC buffers as a list of structs
      duckdb::vector<Value> fn_args {
        Value::LIST({ ValueForIPCBuffer(*ipc_buffer) })
      };

      // Construct a QueryContext to keep the IPC buffer alive
      DuckContext* scan_context = DuckContext::Emplace(
         context_map
        ,std::make_unique<DuckContext>(
            engine_conn.TableFunction("scan_arrow_ipc", fn_args)
           ,ipc_buffer
         )
      );

      return scan_context->uuid;
    }

    //! Create a duckdb scan operator from an arrow file
    size_t EngineDuckDB::ContextForArrowScanOp(fs::path arrow_fpath) {
      // `scan_arrows_file` takes a vector of file paths as input
      duckdb::vector<Value> fn_args {
        Value::LIST({ Value { arrow_fpath } })
      };

      // Construct a QueryContext to keep everything alive
      DuckContext* scan_context = DuckContext::Emplace(context_map
        ,std::make_unique<DuckContext>(
           engine_conn.TableFunction("scan_arrows_file", fn_args)
         )
      );

      return scan_context->uuid;
    }

    //! Registers a context for the table function `execute_mohair`
    //  This table function takes a single binary blob as input, the serialized substrait
    //  plan. The table function is then registered in a context for subsequent access.
    size_t EngineDuckDB::ContextForArrowScanOp(const string& plan_msg) {
      duckdb::vector<Value> fn_args { Value::BLOB_RAW(plan_msg) };
      DuckContext*          scan_context = DuckContext::Emplace(
         context_map
        ,std::make_unique<DuckContext>(
           engine_conn.TableFunction("execute_mohair", fn_args)
         )
      );

      return scan_context->uuid;
    }

    std::tuple<size_t, uint64_t>
    EngineDuckDB::CreateExecutionContext(SystemPlan& sys_plan) {
      // Translate the system plan for execution and register it in a context
      SkytetherStartTS(ActionEngineTranslatePlan);
      DuckContext* ctx = DuckContext::Emplace(
         context_map
        ,std::make_unique<DuckContext>(this->TranslatePlan(sys_plan))
      );
      SkytetherStopTS(ActionEngineTranslatePlan);
      SkytetherLogTimestamps(ActionEngineTranslatePlan);

      return std::make_tuple(ctx->uuid, sys_plan.Hash());
    }

    //! Given a query context ID and a name, create a view from that query
    Status
    EngineDuckDB::ExecuteContext(size_t context_id, uint64_t view_id) {
      string view_name { std::to_string(view_id) };

      SkytetherDebugMsg(
        "Context: ["
           << "(" << std::to_string(context_id) << ") " << view_name
        << "]"
      );

      // Execute the relation and move the result
      DuckContext*       ctx       = GetDuckContext(context_id);
      duck_sptr<DuckRel> query_rel = ctx->duck_plan;

      try {
        engine_conn.Table(INVALID_SCHEMA, view_name);
        ctx->status = QueryStatus::Complete;
        ctx->status_cv.notify_all();
        SkytetherDebugMsg("Execution plan already materialized");
        return Status::OK();
      }
      catch (const duckdb::CatalogException& duck_err) {
        // NOTE: we could do nothing here, but I decided to put this debugging code here
        SkytetherStartTS(ActionEngineExecAnalyze);

        query_rel->context->GetContext()->EnableProfiling();
        ARROW_RETURN_NOT_OK(
          PrintQueryResults(
             *(query_rel->Explain(ExplainType::EXPLAIN_ANALYZE))
            ,0, 10
            ,0, 10
            ,0, 10
          )
        );
        query_rel->context->GetContext()->DisableProfiling();

        SkytetherStopTS(ActionEngineExecAnalyze);
        SkytetherLogTimestamps(ActionEngineExecAnalyze);
      }

      // Create a view that wraps (references) the query
      constexpr bool             is_temporary { true };
      constexpr OnCreateConflict opt_replace  { OnCreateConflict::REPLACE_ON_CONFLICT };
      auto rel_createview = ctx->duck_plan->CreateRel(
        INVALID_SCHEMA, view_name, is_temporary, opt_replace
      );

      // Change the query status, execute the query, then notify when complete
      SkytetherStartTS(ActionEngineExecute);

      ctx->status = QueryStatus::Running;
      rel_createview->Execute();
      {
        lock_guard<mutex> status_lock(ctx->status_mtx);
        ctx->status = QueryStatus::Complete;
      }
      ctx->status_cv.notify_all();

      SkytetherStopTS(ActionEngineExecute);

      SkytetherLogTimestamps(ActionEngineExecute);

      return Status::OK();
    }

    //! Given an ID for a query context, execute that query
    Status
    EngineDuckDB::ExecuteContext(size_t context_id) {
      // Execute the relation and move the result
      DuckContext* ctx = GetDuckContext(context_id);

      ctx->status = QueryStatus::Running;

      std::cout << "Constructing a reader for query result" << std::endl;
      ARROW_ASSIGN_OR_RAISE(
         ctx->result
        ,ReaderForResult(ctx->duck_plan->Execute())
      );

      ctx->status = QueryStatus::Complete;

      return Status::OK();
    }

    //! Given an ID for a query context, return the previously stored relation
    DuckRel* EngineDuckDB::GetRelation(size_t context_id) {
      DuckContext* ctx = GetDuckContext(context_id);

      if (ctx) { return ctx->duck_plan.get(); }
      return nullptr;
    }

    Status
    EngineDuckDB::MaterializeResults(const string& table_name, RecordBatchVector batches) {
      ARROW_ASSIGN_OR_RAISE(auto ipc_buffer, SerializeRecordBatches(std::move(batches)));
      child_list_t<Value> struct_vals {
         { "ptr" , Value::UBIGINT((uintptr_t) ipc_buffer->mutable_data()) }
        ,{ "size", Value::UBIGINT((uint64_t)  ipc_buffer->size())         }
      };

      duckdb::vector<Value> scan_args {
        Value::LIST({ Value::STRUCT(struct_vals) })
      };

      constexpr bool             is_temporary { true };
      constexpr OnCreateConflict opt_replace  { OnCreateConflict::REPLACE_ON_CONFLICT };
      duck_sptr<DuckRel> materialize_plan = (
        engine_conn.TableFunction("scan_arrow_ipc", scan_args)
                   ->CreateRel(INVALID_SCHEMA, table_name, is_temporary, opt_replace)
      );

      auto exec_results = materialize_plan->Execute();
      if (exec_results->HasError()) {
        return Status::Invalid(exec_results->GetError());
      }

      return Status::OK();
    }

    //! Given a query context ID and a name, create a view from that query
    Result<shared_ptr<RecordBatchReader>>
    EngineDuckDB::ScanResults(const string& view_name) {
      duckdb::vector<string>                      proj_aliases;
      duckdb::vector<duck_uptr<ParsedExpression>> proj_exprs;
      proj_exprs.emplace_back(duckdb::make_uniq<StarExpression>());

      duck_sptr<DuckRel> proj_rel = (
        engine_conn.Table(INVALID_SCHEMA, view_name)
                  ->Project(std::move(proj_exprs), std::move(proj_aliases))
      );

      return ReaderForResult(proj_rel->Execute());
    }

  } // namespace: skytether::engines

#endif
