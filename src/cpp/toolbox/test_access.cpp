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


/**
 * A command-line tool that takes a path (absolute or relative) to an arrow file and parses it.
 */

#include <unistd.h>

#include "../headers/skytether.hpp"
#include "../headers/skykinetic.hpp"
#include "../headers/skyhookfs.hpp"


int main(int argc, char **argv) {
    //bool was_plain = false;
    bool    was_table   = false;
    bool    was_meta    = false;
    bool    did_access  = false;

    uint8_t stripe_size = 1;

    Result<shared_ptr<Table>>  table_result;
    Result<shared_ptr<Schema>> schema_result;

    KineticConn kconn = KineticConn();

    char parsed_opt = (char) getopt(argc, argv, "f:k:m:t:s:");
    if (parsed_opt != -1) {
        switch (parsed_opt) {
            case 'f': {
                if (not did_access) {
                    was_table  = true;
                    did_access = true;

                    fs::path path_to_arrow = local_file_protocol + fs::absolute(optarg).string();
                    sky_debug_printf("Parsing file: '%s'\n", path_to_arrow.c_str());

                    // Read arrow table from the given `path_to_arrow`
                    table_result = ReadIPCFile(path_to_arrow.string());
                }

                break;
            }

            case 'k': {
                if (not did_access) {
                    was_table  = true;
                    did_access = true;
                    std::string table_key(optarg);

                    kconn.Connect();
                    if (kconn.IsConnected()) {
                        // We don't know if this is actually a slice, so grab the KBuffer
                        auto get_result = kconn.GetSliceData(table_key, stripe_size);
                        if (not get_result.ok()) {
                            table_result = Result<shared_ptr<Table>>(
                                Status::Invalid("Unable to access partition slice")
                            );
                        }

                        // Construct a RecordBatchReader for the KBuffer
                        auto slice_buffer  = std::move(get_result.ValueOrDie());
                        auto reader_result = slice_buffer->NewRecordBatchReader();
                        if (not reader_result.ok()) {
                            slice_buffer.reset();
                            table_result = Result<shared_ptr<Table>>(
                                Status::Invalid("Unable to construct reader for slice buffer")
                            );
                        }

                        // Try to construct a Table from the reader (which is a shared_ptr)
                        else {
                            auto reader  = reader_result.ValueOrDie();
                            table_result = Table::FromRecordBatchReader(reader.get());
                        }
                    }
                    else {
                        table_result = Result<shared_ptr<Table>>(
                            arrow::Status::Invalid("Could not connect to kinetic drive")
                        );
                    }
                }

                break;
            }

            case 't': {
                if (not did_access) {
                    did_access = true;
                    //was_plain  = true;

                    kconn.Connect();
                    if (not kconn.IsConnected()) { return 1; }

                    auto buffer_result = kconn.GetSliceData(optarg, stripe_size);
                    if (not buffer_result.ok()) { return 1; }

                    auto data_buffer = std::move(buffer_result.ValueOrDie());
                    std::cout << data_buffer->ToString() << std::endl;
                }

                break;
            }

            case 'm': {
                if (not did_access) {
                    did_access = true;
                    was_meta   = true;
                    std::string metadata_key(optarg);

                    kconn.Connect();
                    if (kconn.IsConnected()) {
                        auto meta_result = kconn.GetPartitionMeta(metadata_key);
                        if (meta_result.ok()) {
                            auto pmeta    = meta_result.ValueOrDie();
                            schema_result = Result<shared_ptr<Schema>>(pmeta->schema);
                        }

                        // failed to get data
                        else {
                            schema_result = Result<shared_ptr<Schema>>(
                                arrow::Status::Invalid("Could not get partition metadata")
                            );
                        }
                    }

                    // failed to connect
                    else {
                        schema_result = Result<shared_ptr<Schema>>(
                            arrow::Status::Invalid("Could not connect to kinetic drive")
                        );
                    }
                }

                break;
            }

            case 's': {
                int raw_stripesize = std::stoi(optarg);

                if (raw_stripesize < 0 or raw_stripesize > 255) {
                    std::cerr << "Invalid stripe size; must be in the range [0, 255]"
                              << std::endl
                    ;

                    return 1;
                }

                stripe_size = (uint8_t) raw_stripesize;
                break;
            }

            default: {
                std::cout << "test_access [-s stripe size (default: 1)] "
                          <<            " <[-f path-to-file"
                          <<            " | -k key-name (IPC format)"
                          <<            " | -t key-name (text format)"
                          <<            " | -m meta key-name (IPC format)"
                          <<              "]>"
                          << std::endl
                ;

                std::cout << "NOTE: if '-s' is provided, it must be first"
                          << std::endl
                ;

                return 1;
            }
        }
    }

    if (was_table and not table_result.ok()) {
        std::cerr << "Error: '" << table_result.status() << "'" << std::endl;
        return 1;
    }
    else if (was_meta and not schema_result.ok()) {
        std::cerr << "Error: '" << schema_result.status() << "'" << std::endl;
        return 1;
    }

    else if (was_table) { PrintTable(table_result.ValueOrDie()); }
    else if (was_meta)  { PrintSchema(schema_result.ValueOrDie()); }

    return 0;
}

