// ------------------------------
// License
//
// Copyright 2025 Aldrin Montana
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

#include "skytether.hpp"

using std::vector;

vector<int> IndicesForSelection(int table_colcount) {
  constexpr int col_limit { 3 };

  int sel_size = col_limit;
  if (table_colcount < sel_size) { sel_size = table_colcount; }

  vector<int> col_selection;
  col_selection.reserve(sel_size);
  for (int col_ndx = 0; col_ndx < sel_size; ++col_ndx) {
    col_selection.push_back(col_ndx);
  }

  return col_selection;
}

int main(int argc, char** argv) {
  skytether::fs::path arrow_fpath { skytether::fs::current_path() / "simulated/1_1_1_1_1_500000" };

  std::string arrow_file_uri { "file://" + arrow_fpath.string() };
  auto result_reader = skytether::ReaderForIPCStream(arrow_file_uri);
  if (not result_reader.ok()) {
    skytether::PrintError("Error reading data stream from file", result_reader.status());
    return 1;
  }

  auto batch_reader = std::move(result_reader).ValueOrDie();
  
  auto result_table = skytether::Table::FromRecordBatchReader(batch_reader.get());
  if (not result_table.ok()) {
    skytether::PrintError("Error scanning batches from reader", result_table.status());
    return 2;
  }

  auto table_data = std::move(result_table).ValueOrDie();

  vector<int> col_selection = IndicesForSelection(5);
  auto data_excerpt = table_data->SelectColumns(col_selection);
  if (not data_excerpt.ok()) {
    skytether::PrintError("Error projecting table columns", data_excerpt.status());
    return 9;
  }

  skytether::PrintTable(*data_excerpt, 0, 10);
  return 0;
}
