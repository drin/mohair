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
#pragma once

#include "skytether.hpp"

// >> DuckDB dependencies
#include "duckdb/common/arrow/arrow_wrapper.hpp"
#include "duckdb/function/table/arrow.hpp"
#include "duckdb/planner/table_filter.hpp"


// ------------------------------
// Aliases

namespace skytether {

  using IPCStreamDec = arrow::ipc::StreamDecoder;
  using IPCListener  = arrow::ipc::Listener;

} // namespace: skytether


// ------------------------------
// Classes

namespace skytether {

  //! An IPC Listener that stores the schema and batches of an IPC stream
  struct ArrowIPCStreamBuffer : public IPCListener {
      shared_ptr<Schema> schema_;
      RecordBatchVector  batches_;
      bool is_eos_;

      // Constructors
      ArrowIPCStreamBuffer();

      // Public methods
      bool is_eos() const { return is_eos_;  }

      shared_ptr<Schema>& schema()  { return schema_;  }
      RecordBatchVector&  batches() { return batches_; }

    protected:
      // Protected methods
      Status      OnSchemaDecoded(shared_ptr<Schema>      schema      );
      Status OnRecordBatchDecoded(shared_ptr<RecordBatch> record_batch);
      Status OnEOS();
  };


  struct ArrowRecordBatchReader : public RecordBatchReader {
      ArrowRecordBatchReader(shared_ptr<ArrowIPCStreamBuffer> buffer);

      ~ArrowRecordBatchReader() = default;

      shared_ptr<Schema> schema() const override;

      //! Read the next record batch in the stream. Returns nullptr at the end.
      Status ReadNext(shared_ptr<RecordBatch> *batch) override;

    protected:
      shared_ptr<ArrowIPCStreamBuffer> buffer_;
      size_t                           next_batch_id_;
  };


  //! Exports an ArrowRecordBatchReader on the input IPC buffer to the C data interface
  duck_uptr<ArrowArrayStreamWrapper>
  CStreamForIPCBuffer(uintptr_t buffer_ptr, ArrowStreamParameters &parameters);

  //! Uses an ArrowRecordBatchReader on the input IPC buffer to set schema
  void SchemaFromIPCBuffer( shared_ptr<ArrowIPCStreamBuffer> buffer
                           ,ArrowSchemaWrapper&              schema);

} // namespace: skytether
