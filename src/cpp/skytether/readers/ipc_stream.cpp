// ------------------------------
// License
//
// >> DuckDB License
// Copyright 2018-2024 Stichting DuckDB Foundation
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be included in all copies
// or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// >> Internal License
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

#include "skytether/readers/ipc_stream.hpp"


// ------------------------------
// Class implementations

namespace skytether {

  // >> ArrowIPCStreamBuffer implementations

  ArrowIPCStreamBuffer::ArrowIPCStreamBuffer()
    : schema_(nullptr), batches_(), is_eos_(false) {}

  //! Event handlers (arrow::ipc::Listener interface)
  arrow::Status
  ArrowIPCStreamBuffer::OnSchemaDecoded(shared_ptr<arrow::Schema> s) {
    schema_ = s;
    return arrow::Status::OK();
  }

  arrow::Status
  ArrowIPCStreamBuffer::OnRecordBatchDecoded(shared_ptr<arrow::RecordBatch> batch) {
    batches_.push_back(std::move(batch));
    return arrow::Status::OK();
  }

  arrow::Status
  ArrowIPCStreamBuffer::OnEOS() {
    is_eos_ = true;
    return arrow::Status::OK();
  }


  // >> ArrowRecordBatchReader implementations

  ArrowRecordBatchReader::ArrowRecordBatchReader(shared_ptr<ArrowIPCStreamBuffer> buffer)
      : buffer_(buffer), next_batch_id_(0) {}

  shared_ptr<arrow::Schema>
  ArrowRecordBatchReader::schema() const { return buffer_->schema(); }

  //! Read the next record batch in the stream. Sets *batch to null at end of stream
  arrow::Status
  ArrowRecordBatchReader::ReadNext(shared_ptr<arrow::RecordBatch>* batch) {
    if (next_batch_id_ >= buffer_->batches().size()) {
      *batch = nullptr;
    }

    else {
      *batch = buffer_->batches()[next_batch_id_++];
    }

    return arrow::Status::OK();
  }

  // TODO: use a function like this to feed RecordBatchReader from Flight into ArrowScan
  //! Creates a C stream on the input IPC buffer
  duckdb_uptr<duckdb::ArrowArrayStreamWrapper>
  CStreamForIPCBuffer(uintptr_t buffer_ptr, ArrowStreamParameters &parameters) {
    duckdb::assert(buffer_ptr != 0);

    auto buffer = reinterpret_cast<shared_ptr<ArrowIPCStreamBuffer>*>(buffer_ptr);

    auto reader = std::make_shared<ArrowRecordBatchReader>(*buffer);

    // Create arrow stream
    auto stream_wrapper = duckdb::make_uniq<duckdb::ArrowArrayStreamWrapper>();
    stream_wrapper->arrow_array_stream.release = nullptr;

    // Export the RecordBatchReader to use the C data stream interface
    auto export_status = arrow::ExportRecordBatchReader(
      reader, &stream_wrapper->arrow_array_stream
    );

    if (!export_status.ok()) {
      std::cerr << "Error when exporting ArrowRecordBatchReader:" << std::endl
                << export_status.message()                        << std::endl
      ;
      return nullptr;
    }

    return stream_wrapper;
  }

  //! Sets Arrow schema (C data interface) from input IPC buffer
  void SchemaFromIPCBuffer( shared_ptr<ArrowIPCStreamBuffer> buffer
                           ,duckdb::ArrowSchemaWrapper&           schema) {
    auto reader = std::make_shared<ArrowRecordBatchReader>(buffer);

    // Export RecordBatchReader to C data interface so we get an `ArrowSchema`
    auto stream_wrapper = duckdb::make_uniq<duckdb::ArrowArrayStreamWrapper>();
    stream_wrapper->arrow_array_stream.release = nullptr;

    auto export_status = arrow::ExportRecordBatchReader(
      reader, &stream_wrapper->arrow_array_stream
    );

    if (!export_status.ok()) { return; }

    // Set the `arrow_schema` attribute of ArrowSchemaWrapper (also passes ownership)
    stream_wrapper->arrow_array_stream.get_schema(
       &stream_wrapper->arrow_array_stream
      ,&schema.arrow_schema
    );
  }

} // namespace: skytether
