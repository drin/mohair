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

#include <iostream>
#include <filesystem>

#include "duckdb.hpp"

// Internal duckdb headers that we need
#include "duckdb/common/helper.hpp"

#include "duckdb/main/connection.hpp"
#include "duckdb/main/table_description.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"

#include "duckdb/parser/expression/star_expression.hpp"

#include "duckdb/main/relation.hpp"
#include "duckdb/main/relation/table_relation.hpp"
#include "duckdb/main/relation/view_relation.hpp"
#include "duckdb/main/relation/projection_relation.hpp"


// ------------------------------
// Aliases

// >> namespace aliases
namespace fs = std::filesystem;

// >> type aliases

//  duckdb data types
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


// ------------------------------
// Functions

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
CustomTableLookup( duckdb::shared_ptr<ClientContext>& context
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
// Main

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: duckdb-sandbox <path-to-datafile>" << std::endl;
    return 1;
  }

  // Parse CLI args
  fs::path db_fpath { argv[1] };

  // Create database to play with
  DuckDB     disk_db { db_fpath };
  Connection conn    { disk_db  };

  // Create read query plan
  string duck_defaultschema { "main"          };
  string src_name           { "genes"         };

  // Create a view
  // auto src_entry = CustomTableLookup(conn.context, duck_defaultschema, src_name);
  auto src_entry = conn.TableInfo(duck_defaultschema, src_name);
  shared_ptr<TableRelation> src_rel = duckdb::make_shared_ptr<TableRelation>(
    conn.context, std::move(src_entry)
  );

  vector<unique_ptr<ParsedExpression>> proj_exprs;
  proj_exprs.emplace_back(duckdb::make_uniq<StarExpression>());

  vector<string> proj_aliases;

  shared_ptr<Relation> proj_rel = duckdb::make_shared_ptr<ProjectionRelation>(src_rel, std::move(proj_exprs), std::move(proj_aliases));
  std::cout << "Query plan:"        << std::endl
            << proj_rel->ToString() << std::endl
  ;

  shared_ptr<Relation> view_rel = proj_rel->CreateView(duck_defaultschema, "testview_genes", false, true);
  std::cout << "View plan:"        << std::endl
            << view_rel->ToString() << std::endl
  ;


  // Query the view
  shared_ptr<ViewRelation> src_viewrel = duckdb::make_shared_ptr<ViewRelation>(conn.context, duck_defaultschema, "testview_genes");

  vector<unique_ptr<ParsedExpression>> view_exprs;
  view_exprs.emplace_back(duckdb::make_uniq<StarExpression>());

  vector<string> view_aliases;
  shared_ptr<Relation> proj_viewrel = duckdb::make_shared_ptr<ProjectionRelation>(src_viewrel, std::move(view_exprs), std::move(view_aliases));

  std::cout << "Query plan (from view):" << std::endl
            << proj_viewrel->ToString()  << std::endl
  ;

  // View the query results
  idx_t count_chunks = 2;
  idx_t count_cols   = 10;
  idx_t count_rows   = 10;

  auto query_result = proj_viewrel->Execute();
  return PrintQueryResults(
     *query_result
    ,0, count_chunks
    ,0, count_cols
    ,0, count_rows
  );
}
