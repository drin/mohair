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

#include "adapter_duckdb.hpp"


// ------------------------------
// Only define if DuckDB is enabled
#if USE_DUCKDB

  // ------------------------------
  // Functions

  namespace mohair::adapters {

    Value ValueForIPCBuffer(Buffer& ipc_buffer) {
      // Place values into a child_list_t<type>
      child_list_t<Value> struct_vals {
         { "ptr" , Value::UBIGINT((uintptr_t) ipc_buffer.mutable_data()) }
        ,{ "size", Value::UBIGINT((uint64_t)  ipc_buffer.size())         }
      };

      // Call the STRUCT builder function
      return Value::STRUCT(struct_vals);
    }

    //! Prints `col_count` columns of the given chunk starting at `col_offset`
    void PrintChunk( DataChunk& src_chunk
                    ,idx_t col_offset, idx_t col_count
                    ,idx_t row_offset, idx_t row_count) {
      std::cout << "Chunk - [" << std::to_string(col_count) << " Columns]" << std::endl;

      idx_t col_ndx { col_offset };
      for (; col_ndx < src_chunk.ColumnCount() && col_ndx < col_count; ++col_ndx) {
        idx_t          view_length { row_count - row_offset                          };
        duckdb::Vector col_view    { src_chunk.data[col_ndx],  row_offset, row_count };

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
      duckdb::unique_ptr<DataChunk> result_chunk { nullptr };
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

  } // namespace: mohair::adapters


  // ------------------------------
  // Class Implementations

  namespace mohair::adapters {

    EngineDuckDB DuckDBForMem() {
      DuckDB mem_db;

      return EngineDuckDB(mem_db);
    }


    //! Create a duckdb scan operator from an IPC buffer (extracted from an arrow file)
    Result<int> EngineDuckDB::ArrowScanOpIPC(fs::path arrow_fpath) {
      // Scan table with duckdb by grabbing the whole file as one IPC buffer
      auto buffer_result = BufferFromIPCStream(arrow_fpath);
      if (not buffer_result.ok()) {
        std::cerr << "Unable to parse buffer from IPC stream" << std::endl
                  << "\t" << buffer_result.status().message() << std::endl
        ;

        return Status::Invalid("Unable to parse buffer from IPC stream");
      }

      // Construct a QueryContext to keep everything alive
      auto scan_context = std::make_unique<QueryContext>();

      // Push the IPC buffer into the QueryContext
      auto &ipc_buffer = (
        scan_context->rel_mem.emplace_back(std::move(buffer_result).ValueOrDie())
      );

      // `scan_arrow_ipc` takes IPC buffers as a list of structs
      duckdb::vector<Value> fn_args {
        Value::LIST({ ValueForIPCBuffer(*ipc_buffer) })
      };

      // Get a relation representing a scan of Arrow IPC buffers
      scan_context->duck_rel = engine_conn.TableFunction("scan_arrow_ipc", fn_args);

      int prepared_ctxtid = ++context_id;
      query_contexts[prepared_ctxtid] = std::move(scan_context);

      return prepared_ctxtid;
    }


    //! Create a duckdb scan operator from an arrow file
    Result<int> EngineDuckDB::ArrowScanOpFile(fs::path arrow_fpath) {
      // Construct a QueryContext to keep everything alive
      auto scan_context = std::make_unique<QueryContext>();

      // `scan_arrows_file` takes a vector of file paths as input
      duckdb::vector<Value> fn_args {
        Value::LIST({ Value { arrow_fpath } })
      };

      // Get a relation representing a scan of Arrow IPC buffers
      scan_context->duck_rel = engine_conn.TableFunction("scan_arrows_file", fn_args);

      int prepared_ctxtid = ++context_id;
      query_contexts[prepared_ctxtid] = std::move(scan_context);

      return prepared_ctxtid;
    }


    //! Given an ID for a query context, execute that query
    Status EngineDuckDB::ExecuteRelation(int context_id) {
      // Execute the relation and move the result
      auto& rel_context = query_contexts[context_id];
      rel_context->rel_result = std::move(rel_context->duck_rel->Execute());

      // Peek into the result
      auto& query_result = *(rel_context->rel_result);
      RETURN_NOT_OK(PrintQueryResults(query_result));

      return Status::OK();
    }

  } // namespace: mohair::adapters

#endif
