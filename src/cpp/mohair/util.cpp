// ------------------------------
// License
//
// Copyright 2023 Aldrin Montana
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

#include "mohair.hpp"


// ------------------------------
// Functions

namespace mohair {

  //  >> Reader functions

  /** Given a file path, return a binary input stream. */
  fstream StreamForFile(const char *in_fpath) {
    return fstream { in_fpath, std::ios::in | std::ios::binary };
  }

  /** Given a file path, read the file data as binary into an output string. */
  bool FileToString(const char *in_fpath, string &file_data) {
    // create an IO stream for the file
    auto file_stream = StreamForFile(in_fpath);
    if (!file_stream) {
      std::cerr << "Failed to open IO stream for file" << std::endl;
      return false;
    }

    // go to end of stream, read the position, then reset position
    file_stream.seekg(0, std::ios_base::end);
    auto size = file_stream.tellg();
    file_stream.seekg(0);
    std::cout << "File size: [" << std::to_string(size) << "]" << std::endl;

    // Resize the output and read the file data into it
    file_data.resize(size);
    auto output_ptr = &(file_data[0]);
    file_stream.read(output_ptr, size);

    // On success, the number of characters read will match size
    return file_stream.gcount() == size;
  }


  //  >> Convenience Functions

  /**
   * Join each string in a vector using a given delimiter string literal.
   */
  string JoinStr(vector<string> str_parts, const char *delim) {
    stringstream join_stream;

    join_stream << str_parts[0];
    for (size_t ndx = 1; ndx < str_parts.size(); ++ndx) {
      join_stream << delim << str_parts[ndx];
    }

    return join_stream.str();
  }

  //  >> Debugging Functions

  /** Simple function to print a string literal and an arrow status. */
  void PrintError(const char *msg, const Status arrow_status) {
      std::cerr << msg                             << std::endl
                << "\t" << arrow_status.ToString() << std::endl
      ;
  }

} // namespace: mohair
