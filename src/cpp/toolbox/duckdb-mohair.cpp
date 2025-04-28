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

// >> Standard libs
#include <unistd.h>
#include <iostream>
#include <filesystem>

// >> DuckDB
#include "duckdb.hpp"
#include "duckdb/common/helper.hpp"

#include "duckdb/main/connection.hpp"
#include "duckdb/main/table_description.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"

#include "duckdb/parser/expression/star_expression.hpp"

#include "duckdb/main/relation.hpp"
#include "duckdb/main/relation/table_relation.hpp"
#include "duckdb/main/relation/view_relation.hpp"
#include "duckdb/main/relation/create_view_relation.hpp"
#include "duckdb/main/relation/projection_relation.hpp"

// >> Internal
#include "skytether_cli.hpp"
#include "services/types.hpp"
#include "services/service_skytether.hpp"

#include <arrow/io/api.h>


// ------------------------------
// Aliases

// >> namespace aliases
namespace fs = std::filesystem;

// >> Arrow types
using arrow::Schema;
using arrow::RecordBatch;
using arrow::RecordBatchVector;

// >> DuckDB types
using duckdb::unique_ptr;
using duckdb::shared_ptr;
using duckdb::idx_t;
using duckdb::string;
using duckdb::vector;

//  for interfacing with the database
using duckdb::DuckDB;
using duckdb::Connection;
using duckdb::ClientContext;

using duckdb::Catalog;
using duckdb::TableDescription;
using duckdb::TableCatalogEntry;
using duckdb::OnEntryNotFound;

//  for interfacing with the query engine
using duckdb::Vector;
using duckdb::DataChunk;
using duckdb::ErrorData;
using duckdb::QueryResult;

using duckdb::ParsedExpression;
using duckdb::StarExpression;

using duckdb::Relation;
using duckdb::TableRelation;
using duckdb::ViewRelation;
using duckdb::ProjectionRelation;

// >> Internal Types
using skytether::Status;
using skytether::Buffer;

using skytether::RecordBatchReader;
using skytether::RecordBatchVector;

using skytether::services::Location;
using skytether::services::ResultStream;
using skytether::services::FlightStreamReader;

using skytether::services::FlightStreamChunk;

using skytether::services::SkytetherClient;
using skytether::services::SkytetherTicket;

// >> Functions
using skytether::cli::ParseArgLocationUri;


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "duckdb-mohair"
              << " -l service-location-uri"
              << " -i query-context-id"
              << " [-h]"
              << std::endl
    ;

    return 0;
}


//! Prints `col_count` columns of the given chunk starting at `col_offset`
void PrintChunk( DataChunk& src_chunk
                ,idx_t col_offset, idx_t col_count
                ,idx_t row_offset, idx_t row_count) {
  std::cout << "Chunk - [" << std::to_string(col_count) << " Columns]" << std::endl;

  idx_t col_ndx { col_offset };
  for (; col_ndx < src_chunk.ColumnCount() && col_ndx < col_count; ++col_ndx) {
    idx_t  view_length { row_count - row_offset };
    Vector col_view    { src_chunk.data[col_ndx], row_offset, row_count };

    std::cout << "- " << col_view.ToString(view_length) << std::endl;
  }
}

int PrintQueryResults( QueryResult& result_set
                      ,idx_t chunk_offset, idx_t chunk_count
                      ,idx_t col_offset  , idx_t col_count
                      ,idx_t row_offset  , idx_t row_count) {
  idx_t     chunk_ndx { 0 };
  ErrorData result_err;

  // Grab first chunk
  unique_ptr<DataChunk> result_chunk { nullptr };
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
    std::cerr << "DuckDB: Failed to fetch result chunk " + std::to_string(chunk_ndx) << std::endl;
    return 10;
  }

  return 0;
}

unique_ptr<TableDescription>
CustomTableLookup( shared_ptr<ClientContext>& context
                  ,const string&                      sname
                  ,const string&                      tname) {

  string duck_catalog { "" };

  auto table_entry = Catalog::GetEntry<TableCatalogEntry>(
     *context, duck_catalog, sname, tname
    ,OnEntryNotFound::RETURN_NULL
  );

  if (!table_entry) { return nullptr; }

  auto table_descr = duckdb::make_uniq<TableDescription>(duck_catalog, sname, tname);
  for (auto& tcol : table_entry->GetColumns().Logical()) {
    table_descr->columns.emplace_back(tcol.Copy());
  }

  return table_descr;
}


// ------------------------------
// Structs and Classes

//! A wrapper around FlightStreamReader that provides a RecordBatchReader interface.
struct FlightStreamBatchReader : skytether::RecordBatchReader {

  std::unique_ptr<FlightStreamReader> flight_stream;
  std::shared_ptr<RecordBatch>        current_batch;

  FlightStreamBatchReader(std::unique_ptr<FlightStreamReader> flight_reader)
    : flight_stream(std::move(flight_reader)) {}

  std::shared_ptr<Schema> schema() const override {
    auto result_schema = flight_stream->GetSchema();
    if (not result_schema.ok()) { return nullptr; }

    return std::move(result_schema).ValueOrDie();
  }

  Status ReadNext(std::shared_ptr<RecordBatch>* batch) override {
    ARROW_ASSIGN_OR_RAISE(FlightStreamChunk result_chunk, flight_stream->Next());
    *batch = std::move(result_chunk.data);

    return Status::OK();
  }

};

struct ClientActions {
  size_t            result_id;
  string            result_name;
  Location          service_loc;
  size_t            buffer_size;
  RecordBatchVector result_batches;

  // >> "Application Interface"
  arrow::Result<unique_ptr<duckdb::ArrowArrayStreamWrapper>>
  ArrowStreamForData(SkytetherClient& client_conn, SkytetherTicket& query_ticket) {
    SkytetherDebugMsg("Using FlightStreamReader directly");
    SkytetherDebugMsg(
       "Context: [(" << query_ticket.Id() << ") " << query_ticket.Name() << "]"
    );

    ARROW_ASSIGN_OR_RAISE(
       std::unique_ptr<FlightStreamReader> result_reader
      ,client_conn.GetQueryResults(query_ticket)
    );

    auto stream_reader = std::make_shared<FlightStreamBatchReader>(
      std::move(result_reader)
    );

    // Create arrow stream
    auto stream_wrapper = duckdb::make_uniq<duckdb::ArrowArrayStreamWrapper>();
    stream_wrapper->arrow_array_stream.release = nullptr;

    // Export the RecordBatchReader to use the C data stream interface
    ARROW_RETURN_NOT_OK(
      arrow::ExportRecordBatchReader(
        std::move(stream_reader), &stream_wrapper->arrow_array_stream
      )
    );

    return stream_wrapper;
  }

  arrow::Result<RecordBatchVector>
  ResultsByArrowStream(SkytetherClient& client_conn, SkytetherTicket& query_ticket) {
    ARROW_ASSIGN_OR_RAISE(
       auto stream_wrapper
      ,ArrowStreamForData(client_conn, query_ticket)
    );

    RecordBatchVector result_batches;

    duckdb::ArrowSchemaWrapper schema_wrapper;
    stream_wrapper->GetSchema(schema_wrapper);
    ARROW_ASSIGN_OR_RAISE(
       auto stream_schema
      ,arrow::ImportSchema(&(schema_wrapper.arrow_schema))
    );

    auto chunk = stream_wrapper->GetNextChunk();
    while (chunk->arrow_array.release) {
      if (chunk->arrow_array.length != 0) {
        ARROW_ASSIGN_OR_RAISE(
           auto chunk_batch
          ,arrow::ImportRecordBatch(&(chunk->arrow_array), stream_schema)
        );

        result_batches.push_back(std::move(chunk_batch));
      }

      chunk = stream_wrapper->GetNextChunk();
    }

    return result_batches;
  }

  //! Executes a query by submitting the query plan then fetching the results.
  Status GetQueryResults(SkytetherClient& client_conn) {
    SkytetherDebugMsg(
         "Requesting results for ["
      << std::to_string(result_id) << " >> '"
      << result_name
      << "']"
    );
    SkytetherTicket query_ticket = SkytetherTicket::ForContext(result_id, result_name);

    SkytetherDebugMsg("Requesting results");
    // ARROW_ASSIGN_OR_RAISE(result_batches, RequestResultSet(client_conn, query_ticket));
    ARROW_ASSIGN_OR_RAISE(result_batches, ResultsByArrowStream(client_conn, query_ticket));

    SkytetherDebugMsg(
         "Received results in ["
      << std::to_string(result_batches.size())
      << "] batches"
    );
    if (not result_batches.empty()) {
      skytether::PrintRecordBatch(result_batches[0], 0, 0);
    }

    return Status::OK();
  }


  // >> Public entry point

  //! The entry point for the `ClientActions` struct to send all user requests.
  int SendRequests() {
    // Create and connect a FlightClient
    auto client_conn = SkytetherClient::ForLocation(service_loc);
    if (client_conn == nullptr) { return ERRCODE_CONN_CLIENT; }

    auto status_query = GetQueryResults(*client_conn);
    if (not status_query.ok()) {
      skytether::PrintError("Unable to retrieve query results", status_query);
      return ERRCODE_API_QUERY;
    }

    return 0;
  }

}; // struct: ClientActions


// ------------------------------
// Main Logic

int main(int argc, char **argv) {
  ClientActions client_actions;

  // >> For pulling results
  // Parse each argument and internalize the provided option
  constexpr char  is_done_parsing = -1;
  const     char* opt_template    = "l:i:n:h";
  char            parsed_opt;
  int             errcode_cli;

  while ((parsed_opt = (char) getopt(argc, argv, opt_template)) != is_done_parsing) {
    switch (parsed_opt) {
      case 'h': { return PrintHelp(); }
      case 'l': {
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.service_loc));
        SkytetherCheckErrCode(errcode_cli, "Failed to parse service location");
        break;
      }
      case 'i': {
        client_actions.result_id = std::stoull(string { optarg });
        break;
      }
      case 'n': {
        client_actions.result_name = string { optarg };
        break;
      }
      default: { break; }
    }
  }

  int requests_status = client_actions.SendRequests();
  if (requests_status != 0) { return requests_status; }

  if (client_actions.result_batches.empty()) {
    SkytetherDebugMsg("Received empty result set");
    return -1;
  }

  // >> Convert results into a format we can hand to duckdb
  auto result_bufstream = arrow::io::BufferOutputStream::Create();
  if (not result_bufstream.ok()) {
    skytether::PrintError("Unable to create local buffer", result_bufstream.status());
    return ERRCODE_CLIENT;
  }

  std::shared_ptr<arrow::io::BufferOutputStream> result_stream { result_bufstream.ValueOrDie() };
  auto status_serialize = arrow::ipc::WriteRecordBatchStream(
     client_actions.result_batches
    ,arrow::ipc::IpcWriteOptions::Defaults()
    ,result_stream.get()
  );
  if (not status_serialize.ok()) {
    skytether::PrintError("Unable to serialize query results", status_serialize);
    return ERRCODE_CLIENT;
  }

  auto result_finishstream = result_stream->Finish();
  if (not result_finishstream.ok()) {
    skytether::PrintError("Unable to finish local buffer", result_finishstream.status());
    return ERRCODE_CLIENT;
  }

  std::shared_ptr<arrow::Buffer> result_ipc = std::move(result_finishstream).ValueOrDie();

  // >> For local execution
  // Create database to play with
  DuckDB     mem_db;
  Connection conn { mem_db };

  string duck_defaultschema { "main" };
  string view_name          { "localview-" + client_actions.result_name };

  // Construct an Arrow Scan operator
  duckdb::child_list_t<duckdb::Value> struct_vals {
     { "ptr" , duckdb::Value::UBIGINT((uintptr_t) result_ipc->mutable_data()) }
    ,{ "size", duckdb::Value::UBIGINT((uint64_t)  result_ipc->size())         }
  };

  vector<duckdb::Value> scan_args {
    duckdb::Value::LIST({
      duckdb::Value::STRUCT(struct_vals)
    })
  };

  // Materialize into local engine
  shared_ptr<duckdb::CreateViewRelation> materialize_plan = (
    duckdb::make_shared_ptr<duckdb::CreateViewRelation>(
       conn.TableFunction("scan_arrow_ipc", scan_args)
      ,duck_defaultschema
      ,view_name
      ,/*replace=*/true
      ,/*temporary=*/true
    )
  );

  unique_ptr<QueryResult> result_materialize { materialize_plan->Execute() };
  result_materialize->Print();

  // Test that we can scan the view
  shared_ptr<ViewRelation> view_rel = (
    duckdb::make_shared_ptr<ViewRelation>(
      conn.context, duck_defaultschema, view_name 
    )
  );

  vector<unique_ptr<ParsedExpression>> proj_exprs;
  proj_exprs.emplace_back(duckdb::make_uniq<StarExpression>());

  vector<string> proj_aliases;

  shared_ptr<Relation> proj_rel = (
    duckdb::make_shared_ptr<ProjectionRelation>(
      view_rel, std::move(proj_exprs), std::move(proj_aliases)
    )
  );

  std::cout << "Query plan:"        << std::endl
            << proj_rel->ToString() << std::endl
  ;

  // View the query results
  idx_t count_chunks = 2;
  idx_t count_cols   = 10;
  idx_t count_rows   = 10;

  auto query_result = proj_rel->Execute();
  return PrintQueryResults(
     *query_result
    ,0, count_chunks
    ,0, count_cols
    ,0, count_rows
  );
}
