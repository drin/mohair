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
#include "../headers/connectors.hpp"


#if USE_BOOSTFS == 1
	namespace fs = boost::filesystem;
#else
	namespace fs = std::filesystem;
#endif


arrow::Result<TablePtr>
SelectFromPartition(std::string p_name, ) {
}


int main(int argc, char **argv) {
    bool was_table       = false;
    bool was_meta        = false;
    //bool was_plain       = false;
    bool did_access      = false;

    arrow::Result<TablePtr>  table_result;

    KineticConn kconn = KineticConn();

    char parsed_opt = (char) getopt(argc, argv, "f:k:m:s:");
    if (parsed_opt != -1) {
        switch (parsed_opt) {
            case 'k': {
                if (not did_access) {
                    was_table  = true;
                    did_access = true;
                    std::string table_key(optarg);

                    kconn.Connect();
                    if (kconn.IsConnected()) {
                        table_result = kconn.GetPartitionSlice(table_key);
                    }
                    else {
                        table_result = arrow::Result<TablePtr>(
                            arrow::Status::Invalid("Could not connect to kinetic drive")
                        );
                    }
                }

                break;
            }

            case 's': {
                if (not did_access) {
                    did_access = true;
                    //was_plain  = true;

                    kconn.Connect();
                    if (not kconn.IsConnected()) { return 1; }

                    arrow::Result<KBuffer*> buffer_result = kconn.GetKey(optarg);
                    if (not buffer_result.ok()) { return 1; }

                    std::cout << buffer_result.ValueOrDie()->ToString()
                              << std::endl
                    ;

                    buffer_result.ValueOrDie()->Release();
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
                        schema_result = kconn.GetPartitionMeta(metadata_key);
                    }
                    else {
                        schema_result = arrow::Result<SchemaPtr>(
                            arrow::Status::Invalid("Could not connect to kinetic drive")
                        );
                    }
                }

                break;
            }

            default: {
                std::cout << "test_access <[-f path-to-file"
                          <<            " | -k key-name"
                          <<            " | -m metadata-key-name"
                          <<              "]>"
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

