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
    unique_ptr<EngineDuckDB> DuckDBForMem() {
      DuckDB mem_db;
      return std::make_unique<EngineDuckDB>(mem_db);
    }

    //! Instantiate a wrapper around a file-backed DuckDB database
    unique_ptr<EngineDuckDB> DuckDBForFile(fs::path db_fpath) {
      DuckDB disk_db(db_fpath);
      return std::make_unique<EngineDuckDB>(disk_db);
    }

    //! Entry point into translating a `SystemPlan` into an `EnginePlan` for DuckDB
    unique_ptr<EnginePlan>
    FromSystemPlan(EngineDuckDB& engine, SystemPlan& sys_plan) {
      return std::make_unique<DuckPlan>(engine.TranslatePlan(sys_plan));
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
    Result<shared_ptr<RecordBatchReader>>
    ReaderForResult(duck_uptr<QueryResult> result_set) {
      // TODO: parameterize
      constexpr size_t result_batchsize { 2048 };

      // ArrowArrayStream keeps a reference to the wrapper
      auto stream_wrapper = new ResultArrowArrayStreamWrapper(
        std::move(result_set), result_batchsize
      );

      // Caller's responsibility to eventually call `release()` from the ArrowArrayStream*
      return arrow::ImportRecordBatchReader(&(stream_wrapper->stream));
    }

    //! Convenience function to get a DuckContext from `ctx_map`
    DuckContext* GetDuckContext(ContextMap& ctx_map, int32_t ctx_id) {
      return dynamic_cast<DuckContext*>(ctx_map.GetContext(ctx_id));
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

    // >> EngineDuckDB
    //! Create a duckdb scan operator from an IPC buffer (extracted from an arrow file)
    int32_t EngineDuckDB::ArrowScanOpIPC(shared_ptr<Buffer> ipc_buffer) {
      // `scan_arrow_ipc` takes IPC buffers as a list of structs
      duckdb::vector<Value> fn_args {
        Value::LIST({ ValueForIPCBuffer(*ipc_buffer) })
      };

      // Construct a QueryContext to keep the IPC buffer alive
      auto scan_context = std::make_unique<DuckContext>(
         engine_conn.TableFunction("scan_arrow_ipc", fn_args)
        ,ipc_buffer
      );

      return context_map.RegisterContext(std::move(scan_context));
    }

    //! Create a duckdb scan operator from an arrow file
    int32_t EngineDuckDB::ArrowScanOpFile(fs::path arrow_fpath) {
      // `scan_arrows_file` takes a vector of file paths as input
      duckdb::vector<Value> fn_args {
        Value::LIST({ Value { arrow_fpath } })
      };

      // Construct a QueryContext to keep everything alive
      auto scan_context = std::make_unique<DuckContext>(
        engine_conn.TableFunction("scan_arrows_file", fn_args)
      );

      return context_map.RegisterContext(std::move(scan_context));
    }

    //! Create a duckdb query plan from a substrait plan message
    int32_t EngineDuckDB::ExecContextForSubstrait(std::string plan_msg) {
      SkytetherDebugMsg("Creating execution context for query plan");

      // for debug purposes
      unique_ptr<mohair::Plan> plan_payload = mohair::SubstraitPlanFromString(plan_msg);
      SkytetherDebugMsg("received payload:");
      mohair::PrintSubstraitPlan(plan_payload.get());

      // `from_substrait` takes a single binary blob as input
      duckdb::vector<Value> fn_args { Value::BLOB_RAW(plan_msg) };

      // Get a relation representing the execution of the substrait plan
      // Construct a QueryContext to keep everything alive
      auto scan_context = std::make_unique<DuckContext>(
        engine_conn.TableFunction("execute_mohair", fn_args)
      );

      return context_map.RegisterContext(std::move(scan_context));
    }

    //! Given an ID for a query context, execute that query
    Status
    EngineDuckDB::ExecuteFromContext(int32_t context_id) {
      // Execute the relation and move the result
      DuckContext* ctx = GetDuckContext(context_map, context_id);

      ctx->status = QueryStatus::Running;

      std::cout << "Constructing a reader for query result" << std::endl;
      ARROW_ASSIGN_OR_RAISE(ctx->result, ReaderForResult(ctx->duck_rel->Execute()));

      ctx->status = QueryStatus::Complete;

      return Status::OK();
    }

    //! Given an ID for a query context, return the previously stored relation
    DuckRel* EngineDuckDB::GetRelation(int32_t context_id) {
      DuckContext* ctx = GetDuckContext(context_map, context_id);

      if (ctx) { return ctx->duck_rel.get(); }
      return nullptr;
    }

  } // namespace: skytether::engines

#endif
