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


#if USE_BOOSTFS == 1
	namespace fs = boost::filesystem;
#else
	namespace fs = std::filesystem;
#endif


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("parse-arrow <path-to-arrow-file>\n");
        return 1;
    }

    fs::path path_to_arrow = local_file_protocol + fs::absolute(argv[1]).string();
    sky_debug_printf("Parsing file: '%s'\n", path_to_arrow.c_str());

    // Create a RecordBatchStreamReader for the given `path_to_arrow`
    arrow::Result<TablePtr> read_result = ReadIPCFile(path_to_arrow.string());
    if (not read_result.ok()) { return 1; }

    PrintTable(read_result.ValueOrDie(), 0, 10);

    return 0;
}

