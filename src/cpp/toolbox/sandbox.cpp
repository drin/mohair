/**
 * Copyright 2023 Aldrin Montana

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * This file is purely for testing things with arrow
 */

#include "../headers/skytether.hpp"
#include "../headers/skykinetic.hpp"
#include "../headers/skyhookfs.hpp"
#include "../headers/operators.hpp"


const int NUM_ROWS = 29;
const int NUM_COLS = 105013;


template<typename ElementType, typename BuilderType>
Result<shared_ptr<Array>>
VecToArrow(const std::vector<ElementType> &src_data) {
    BuilderType array_builder;
    ARROW_RETURN_NOT_OK(array_builder.Resize(src_data.size()));
    ARROW_RETURN_NOT_OK(array_builder.AppendValues(src_data));

    shared_ptr<Array> vec_as_arrow;
    ARROW_RETURN_NOT_OK(array_builder.Finish(&vec_as_arrow));

    return Result<shared_ptr<Array>>(vec_as_arrow);
}


Result<shared_ptr<Array>>
MakeArray(int start_val, int num_vals) {
    std::vector<int> array_vals(num_vals, 0 );
    for (size_t vec_ndx = 0; vec_ndx < (size_t) num_vals; vec_ndx++) {
        array_vals[vec_ndx] = start_val + vec_ndx;
    }

    return VecToArrow<int, arrow::Int32Builder>(array_vals);
}


shared_ptr<Schema>
MakeWideSchema() {
    std::vector<shared_ptr<Field>> field_vec(NUM_COLS);

    for (int col_ndx = 0; col_ndx < NUM_COLS; col_ndx++) {
        field_vec[col_ndx] = field(
             std::string("test-field-" + std::to_string(col_ndx))
            ,arrow::uint16()
        );
    }

    return schema(field_vec);
}


shared_ptr<Table>
MakeWideTable() {
    std::vector<shared_ptr<Array>> column_data(NUM_COLS);

    int start_val = 1;
    for (int col_ndx = 0; col_ndx < NUM_COLS; col_ndx++) {
        auto array_result = MakeArray(start_val, NUM_ROWS);
        if (not array_result.ok()) {
            std::cout << "Array construction failed"             << std::endl
                      << "\t" << array_result.status().message() << std::endl
            ;

            return nullptr;
        }

        column_data[col_ndx]  = array_result.ValueOrDie();
        start_val            += NUM_ROWS;
    }

    return arrow::Table::Make(
         MakeWideSchema()
        ,column_data
        ,NUM_ROWS
    );
}


int main(int argc, char **argv) {
    /*
    auto wide_table   = MakeWideTable();
    auto count_result = RowCountForByteSize(wide_table, 1048576, NUM_ROWS);
    if (not count_result.ok()) {
        std::cout << "Failed to determine a row batch size: " << std::endl
                  << "\t" << count_result.status().ToString()
                  << std::endl
        ;

        return 1;
    }

    std::cout << "Recommended row batch size: " << count_result.ValueOrDie() << std::endl;
    */

    KineticConn kconn;
    kconn.Connect();

    if (kconn.IsConnected()) {
        // gene expression
        auto data_result = kconn.GetPartitionData("ebi-test/E-GEOD-98556");
        if (not data_result.ok()) { return 1; }
        auto data_table = data_result.ValueOrDie()->AsTable().ValueOrDie();

        // cluster membership to filter gene expression
        auto cluster_result = kconn.GetPartitionData("ebi-test/E-GEOD-98556.clusters");
        if (not cluster_result.ok()) { return 1; }
        auto cluster_table = cluster_result.ValueOrDie()->AsTable().ValueOrDie();

        std::cout << "Expression (excerpt)" << std::endl;
        PrintTable(data_table, 0, 5);

        std::cout << "Clustered single-cells (excerpt)" << std::endl;
        PrintTable(cluster_table, 0, 5);

        auto member_result = SelectCellsByCluster(cluster_table, { "E-GEOD-98556-0" });
        if (not member_result.ok()) { return 1; }

        auto selection_result = TakeTableColumns(
             data_table
            ,member_result.ValueOrDie()
        );
        if (not selection_result.ok()) { return 1; }

        std::cout << "Filtered expression (excerpt)" << std::endl;
        PrintTable(selection_result.ValueOrDie(), 0, 5);
    }

    /*
    shared_ptr<RecordBatch> curr_recordbatch;
    TableBatchReader        batch_reader(*table_partition);
    int                     batch_ndx;
    batch_reader.set_chunksize(count_result.ValueOrDie());

    auto read_status = batch_reader.ReadNext(&curr_recordbatch);
    for (batch_ndx = 0; read_status.ok() and curr_recordbatch != nullptr; batch_ndx++) {
        // Write data to buffer
        auto write_result = WriteRecordBatchToBuffer(table_partition->schema(), curr_recordbatch);
        if (not write_result.ok()) {
            std::cout << "Failed to write batch to buffer: " << std::endl
                      << "\t" << write_result.status().ToString()
                      << std::endl
            ;

            return 1;
        }

        // Write buffer to file
        std::string output_filepath { output_prefix + std::to_string(batch_ndx) + arrow_ext };
        auto write_status = WriteBufferToFile(write_result.ValueOrDie(), output_filepath);
        if (not write_status.ok()) {
            std::cout << "Failed to write table data: " << std::endl
                      << "\t" << write_status.ToString()
                      << std::endl
            ;

            return 1;
        }

        // read next batch from table
        read_status = batch_reader.ReadNext(&curr_recordbatch);
    }

    if (not read_status.ok() or (batch_ndx == 0 and curr_recordbatch == nullptr)) {
        std::cout << "Failed to read batch from table: " << std::endl
                  << "\t" << read_status.ToString()
                  << std::endl
        ;

        return 1;
    }
    */


    return 0;
}
