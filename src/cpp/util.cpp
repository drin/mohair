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

#include "skytether.hpp"


// ------------------------------
// Type aliases

using IPCReadOpts = arrow::ipc::IpcReadOptions;


// ------------------------------
// Functions

namespace skytether {

  // Anonymous namespace for internal functions
  namespace {

    /** Given a file path, return an arrow::io::ReadableFile. */
    Result<shared_ptr<RandomAccessFile>> ReadHandleForIPCFile(const std::string &path_as_uri) {
      std::string fpath;

      // get a `FileSystem` instance (local fs scheme is "file://")
      std::cout << "Creating read handle for file: " << path_as_uri << std::endl;
      ARROW_ASSIGN_OR_RAISE(auto localfs, arrow::fs::FileSystemFromUri(path_as_uri, &fpath));

      // use the `FileSystem` instance to open a handle to the file
      return localfs->OpenInputFile(fpath);
    }

    /** Given a file path, return an arrow::io::OutputStream. */
    Result<shared_ptr<ArrowOutputStream>> WriteHandleForIPCFile(const std::string &path_as_uri) {
      std::string fpath;

      // get a `FileSystem` instance (local fs scheme is "file://")
      std::cout << "Creating write handle for file: " << path_as_uri << std::endl;
      ARROW_ASSIGN_OR_RAISE(auto localfs, arrow::fs::FileSystemFromUri(path_as_uri, &fpath));

      // use the `FileSystem` instance to open a handle to the file
      return localfs->OpenOutputStream(fpath);
    }

    /** Given a file path, create a RecordBatchStreamReader. */
    Result<shared_ptr<RecordBatchStreamReader>>
    ReaderForIPCStream(const std::string &path_as_uri) {
      std::cout << "Creating reader for IPC stream" << std::endl;

      // use the `FileSystem` instance to open a handle to the file
      ARROW_ASSIGN_OR_RAISE(auto input_file_handle, ReadHandleForIPCFile(path_as_uri));

      // read from the handle using `RecordBatchStreamReader`
      return RecordBatchStreamReader::Open(input_file_handle, IPCReadOpts::Defaults());
    }

    /** Given a file path, create a RecordBatchFileReader. */
    Result<shared_ptr<RecordBatchFileReader>>
    ReaderForIPCFile(const std::string &path_as_uri) {
      std::cout << "Creating reader for IPC file" << std::endl;

      // use the `FileSystem` instance to open a handle to the file
      ARROW_ASSIGN_OR_RAISE(auto input_file_handle, ReadHandleForIPCFile(path_as_uri));

      // read from the handle using `RecordBatchStreamReader`
      return RecordBatchFileReader::Open(input_file_handle, IPCReadOpts::Defaults());
    }

  } // anonymous namespace: skytether::<anonymous>

  // >> Logger functions
  string PathForInstantiatedLog(const string& logger_name) {
    const string path_prefix { "skytether." + logger_name + "." };
    const string path_suffix { ".log"                           };

    auto     ts_logstart = system_clock::to_time_t(system_clock::now());
    std::tm* local_ts    = std::localtime(&ts_logstart);

    stringstream ss;
    ss << path_prefix << std::put_time(local_ts, "%Y%m%d%H%M%S") << path_suffix;

    return ss.str();
  }

  std::fstream* SkytetherLogger() {
    static string empty_name;
    return SkytetherLogger(empty_name);
  }

  std::fstream* SkytetherLogger(string logger_name) {
    static bool         is_initialized { false };
    static std::fstream log_handle;

    if (not is_initialized) {
      string log_fpath = PathForInstantiatedLog(logger_name);
      log_handle       = OutputStreamForFile(log_fpath.data());

      auto ts_init = steady_clock::now();
      log_handle << "[" << mohair::StringifyTS(ts_init) << ":µs] "
                 << "|> initial timestamp"              << std::endl
      ;

      is_initialized = true;
    }

    return &log_handle;
  }

  // >> Reader functions

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
    SkytetherDebugMsg("Reading file: '" << fpath << "'");

    string file_data;
    if (not skytether::FileToString(fpath, file_data)) {
      return Status::Invalid("Failed to parse file data into string");
    }

    return Buffer::FromString(file_data);
  }


  /** Given a file path to an Arrow IPC stream, return the data as a buffer. */
  Result<shared_ptr<Buffer>> BufferFromIPCStream(const std::string& fpath) {
    SkytetherDebugMsg("Parsing file: " << fpath);

    // use the `FileSystem` instance to open a handle to the file
    ARROW_ASSIGN_OR_RAISE(auto arrow_fhandle, ReadHandleForIPCFile(fpath));

    // get the size of the file handle (arrow::io::RandomAccessFile) and read it whole
    ARROW_ASSIGN_OR_RAISE(auto  arrow_fsize, arrow_fhandle->GetSize());
    ARROW_ASSIGN_OR_RAISE(auto arrow_buffer, arrow_fhandle->ReadAt(0, arrow_fsize));

    if (not arrow_buffer->is_cpu()) {
      return Status::Invalid("Read IPC stream file into memory but it is not CPU-accessible");
    }

    SkytetherDebugMsg("Returning IPC buffer");
    return arrow_buffer;
  }

  /** Given a file path to an Arrow IPC stream, return a Table. */
  Result<shared_ptr<Table>> ReadIPCStream(const std::string& fpath) {
    SkytetherDebugMsg("Parsing file: " << fpath);

    // Declares and initializes `batch_reader`
    ARROW_ASSIGN_OR_RAISE(auto batch_reader, ReaderForIPCStream(fpath));

    return Table::FromRecordBatchReader(batch_reader.get());
  }

  /** Given a file path to an Arrow IPC file, return a Table. */
  Result<shared_ptr<Table>> ReadIPCFile(const std::string& fpath) {
    SkytetherDebugMsg("Reading file: " << fpath.data());

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

    return Table::FromRecordBatches(ipc_file_reader->schema(), batches);
  }

  /** Given a file path and Table, write data as an Arrow IPC stream. */
  Status WriteIPCStream(const std::string& path_as_uri, const Table& data_table) {
    // use the `FileSystem` instance to open a handle to the file
    ARROW_ASSIGN_OR_RAISE(auto output_file_handle, WriteHandleForIPCFile(path_as_uri));

    SkytetherDebugMsg("Creating stream writer for file: " << path_as_uri);
    ARROW_ASSIGN_OR_RAISE(
       auto batch_writer
      ,arrow::ipc::MakeStreamWriter(output_file_handle.get(), data_table.schema())
    );

    return batch_writer->WriteTable(data_table);
  }

  /** Given a file path and Table, write data as an Arrow IPC file. */
  Status WriteIPCFile(const std::string& path_as_uri, const Table& data_table) {
    // use the `FileSystem` instance to open a handle to the file
    ARROW_ASSIGN_OR_RAISE(auto output_file_handle, WriteHandleForIPCFile(path_as_uri));

    SkytetherDebugMsg("Creating stream writer for file: " << path_as_uri);
    ARROW_ASSIGN_OR_RAISE(
       auto batch_writer
      ,arrow::ipc::MakeFileWriter(output_file_handle.get(), data_table.schema())
    );

    return batch_writer->WriteTable(data_table);
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
  void PrintSchemaMetadata(shared_ptr<KVMetadata> schema_meta, int64_t offset, int64_t length) {
    // grab a reference to the metadata for convenience
    int64_t metakey_count = schema_meta->size();
    int64_t max_keyndx    = metakey_count;

    std::cout << "Schema Metadata excerpt ";
    if (length > 0) {
      max_keyndx = length < metakey_count ? length : metakey_count;
      std::cout << "(" << max_keyndx << " of " << metakey_count << ")";
    }
    else {
      std::cout << "(" << metakey_count << " of " << metakey_count << ")";
    }
    std::cout << std::endl << "--------------" << std::endl;

    // skytether specific metadata
    Result<size_t> pcount_result = Skytether::GetPartitionCount(schema_meta);
    if (pcount_result.ok()) {
      std::cout << "\tdecoded partition count: " << std::to_string(*pcount_result)
                << std::endl
      ;
    }
    else {
      std::cerr << "\tcould not decode partition count." << std::endl;
    }

    Result<uint8_t> ssize_result = Skytether::GetStripeSize(schema_meta);
    if (ssize_result.ok()) {
      std::cout << "\tdecoded stripe size: " << std::to_string(*ssize_result)
                << std::endl
      ;
    }

    // any other metadata
    for (int64_t meta_ndx = offset; meta_ndx < max_keyndx; meta_ndx++) {
      std::cout << "\t[" << meta_ndx << "] "
                << schema_meta->key(meta_ndx)
                << " -> "
                << schema_meta->value(meta_ndx)
                << std::endl
      ;
    }
  }

  void PrintSchemaAttributes(shared_ptr<Schema> schema, int64_t offset, int64_t length) {
    bool    show_field_meta = true;
    int64_t field_count     = schema->num_fields();
    int64_t max_fieldndx    = field_count;

    std::cout << "Schema Excerpt ";
    if (length > 0) {
      max_fieldndx = length < field_count ? length : field_count;
      std::cout << "(" << max_fieldndx << " of " << field_count << ")";
    }
    else {
      std::cout << "(" << field_count << " of " << field_count << ")";
    }
    std::cout << std::endl << "--------------" << std::endl;

    for (int field_ndx = offset; field_ndx < max_fieldndx; field_ndx++) {
      shared_ptr<Field> schema_field = schema->field(field_ndx);
      std::cout << "\t[" << field_ndx << "]:" << std::endl;
      std::cout << "\t\t"
                << schema_field->ToString(show_field_meta)
                << std::endl
      ;
    }
  }

  //! Print an Arrow Schema to stdout given an offset and length (row count).
  void PrintSchema(shared_ptr<Schema> schema, int64_t offset, int64_t length) {
    std::cout << "Schema:" << std::endl;

    // >> Print some attributes (columns)
    PrintSchemaAttributes(schema, offset, length);

    // >> Print some metadata key-values (if there are any)
    if (schema->HasMetadata()) {
      PrintSchemaMetadata(schema->metadata()->Copy(), offset, length);
    }
  }

  //! Print an Arrow Table to stdout given an offset and length (row count).
  void PrintTable(shared_ptr<Table> table_data, int64_t offset, int64_t length) {
    shared_ptr<Table> table_slice;
    int64_t           row_count { table_data->num_rows() };

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

  //! Print an Arrow RecordBatch to stdout given an offset and length (row count).
  void PrintRecordBatch(shared_ptr<RecordBatch> batch_data, int64_t offset, int64_t length) {
    shared_ptr<RecordBatch> batch_slice;
    int64_t                 row_count { batch_data->num_rows() };

    std::cout << "RecordBatch Excerpt ";

    if (length > 0) {
      int64_t max_rowndx = length < row_count ? length : row_count;
      batch_slice = batch_data->Slice(offset, max_rowndx);
      std::cout << "(" << max_rowndx << " of " << row_count << ")";
    }

    else {
      batch_slice = batch_data->Slice(offset);
      std::cout << "(" << row_count << " of " << row_count << ")";
    }

    std::cout << std::endl
              << "--------------" << std::endl
              << batch_slice->ToString()
              << std::endl
    ;
  }


  /** Simple function to print a string literal and an arrow status. */
  void PrintError(const char *msg, const Status& arrow_status) {
      std::cerr << msg                              << std::endl
                << ":\t" << arrow_status.ToString() << std::endl
      ;
  }

} // namespace: skytether
