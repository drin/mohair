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
// Type aliases

using IPCReadOpts = arrow::ipc::IpcReadOptions;


// ------------------------------
// Functions

namespace mohair {

  // Anonymous namespace for internal functions
  namespace {

    /** Given a file path, return an arrow::ReadableFile. */
    Result<shared_ptr<RandomAccessFile>> HandleForIPCFile(const std::string &path_as_uri) {
      std::cout << "Creating handle for arrow IPC-formatted file: "
                << path_as_uri
                << std::endl
      ;

      std::string fpath;

      // get a `FileSystem` instance (local fs scheme is "file://")
      ARROW_ASSIGN_OR_RAISE(
         auto localfs
        ,arrow::fs::FileSystemFromUri(path_as_uri, &fpath)
      );

      // use the `FileSystem` instance to open a handle to the file
      return localfs->OpenInputFile(fpath);
    }

    /** Given a file path, create a RecordBatchStreamReader. */
    Result<shared_ptr<RecordBatchStreamReader>>
    ReaderForIPCStream(const std::string &path_as_uri) {
      std::cout << "Creating reader for IPC stream" << std::endl;

      // use the `FileSystem` instance to open a handle to the file
      ARROW_ASSIGN_OR_RAISE(auto input_file_handle, HandleForIPCFile(path_as_uri));

      // read from the handle using `RecordBatchStreamReader`
      return RecordBatchStreamReader::Open(input_file_handle, IPCReadOpts::Defaults());
    }

    /** Given a file path, create a RecordBatchFileReader. */
    Result<shared_ptr<RecordBatchFileReader>>
    ReaderForIPCFile(const std::string &path_as_uri) {
      std::cout << "Creating reader for IPC file" << std::endl;

      // use the `FileSystem` instance to open a handle to the file
      ARROW_ASSIGN_OR_RAISE(auto input_file_handle, HandleForIPCFile(path_as_uri));

      // read from the handle using `RecordBatchStreamReader`
      return RecordBatchFileReader::Open(input_file_handle, IPCReadOpts::Defaults());
    }

  } // anonymous namespace: mohair::<anonymous>

  //  >> Reader functions

  /** Given a file path, return a binary input stream. */
  fstream InputStreamForFile(const char* in_fpath) {
    return fstream { in_fpath, std::ios::in | std::ios::binary };
  }

  /** Given a file path, return a binary output stream. */
  fstream OutputStreamForFile(const char* out_fpath) {
    return fstream { out_fpath, std::ios::out | std::ios::trunc | std::ios::binary };
  }

  /** Given a file path, read the file data as binary into an output string. */
  bool FileToString(const char* in_fpath, string& file_data) {
    // create an IO stream for the file
    auto file_stream = InputStreamForFile(in_fpath);
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


  Result<shared_ptr<Buffer>> BufferFromFile(const char* fpath) {
    MohairDebugMsg("Reading file: '" << fpath << "'");

    string file_data;
    if (not mohair::FileToString(fpath, file_data)) {
      return Status::Invalid("Failed to parse file data into string");
    }

    return Buffer::FromString(file_data);
  }


  /** Given a file path to an Arrow IPC stream, return the data as a buffer. */
  Result<shared_ptr<Buffer>> BufferFromIPCStream(const std::string& fpath) {
    MohairDebugMsg("Parsing file: " << fpath);

    // use the `FileSystem` instance to open a handle to the file
    ARROW_ASSIGN_OR_RAISE(auto arrow_fhandle, HandleForIPCFile(fpath));

    // get the size of the file handle (arrow::io::RandomAccessFile) and read it whole
    ARROW_ASSIGN_OR_RAISE(auto  arrow_fsize, arrow_fhandle->GetSize());
    ARROW_ASSIGN_OR_RAISE(auto arrow_buffer, arrow_fhandle->ReadAt(0, arrow_fsize));

    if (not arrow_buffer->is_cpu()) {
      return Status::Invalid("Read IPC stream file into memory but it is not CPU-accessible");
    }

    MohairDebugMsg("Returning IPC buffer");
    return arrow_buffer;
  }

  /** Given a file path to an Arrow IPC stream, return a Table. */
  Result<shared_ptr<Table>> ReadIPCStream(const std::string& fpath) {
    MohairDebugMsg("Parsing file: " << fpath);

    // Declares and initializes `batch_reader`
    ARROW_ASSIGN_OR_RAISE(auto batch_reader, ReaderForIPCStream(fpath));

    return arrow::Table::FromRecordBatchReader(batch_reader.get());
  }

  /** Given a file path to an Arrow IPC file, return a Table. */
  Result<shared_ptr<Table>> ReadIPCFile(const std::string& fpath) {
    MohairDebugMsg("Reading file: " << fpath.data());

    // Declares and initializes `ipc_file_reader`
    ARROW_ASSIGN_OR_RAISE(auto ipc_file_reader, ReaderForIPCFile(fpath));

    // Based on RecordBatchFileReader::ToTable (Arrow >12.0.1)
    // https://github.com/apache/arrow/blob/main/cpp/src/arrow/ipc/reader.h#L236-L237
    RecordBatchVector batches;

    const auto batch_count = ipc_file_reader->num_record_batches();
    for (int batch_ndx = 0; batch_ndx < batch_count; ++batch_ndx) {
      ARROW_ASSIGN_OR_RAISE(auto batch, ipc_file_reader->ReadRecordBatch(batch_ndx));
      batches.emplace_back(batch);
    }

    return arrow::Table::FromRecordBatches(ipc_file_reader->schema(), batches);
  }


  // >> Convenience Functions

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

  /** Print an Arrow Table to stdout given an offset and length (row count). */
  void PrintTable(shared_ptr<Table> table_data, int64_t offset, int64_t length) {
    shared_ptr<Table> table_slice;
    int64_t           row_count = table_data->num_rows();

    std::cout << "Table Excerpt ";

    if (length > 0) {
      int64_t max_rowndx = length < row_count ? length : row_count;
      table_slice = table_data->Slice(offset, max_rowndx);
      std::cout << "(" << max_rowndx << " of " << row_count << ")";
    }

    else {
      table_slice = table_data->Slice(offset);
      std::cout << "(" << row_count << " of " << row_count << ")";
    }

    std::cout << std::endl
              << "--------------" << std::endl
              << table_slice->ToString()
              << std::endl
    ;
  }


  /** Simple function to print a string literal and an arrow status. */
  void PrintError(const char *msg, const Status& arrow_status) {
      std::cerr << msg                             << std::endl
                << "\t" << arrow_status.ToString() << std::endl
      ;
  }

} // namespace: mohair
