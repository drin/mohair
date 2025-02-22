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


// ------------------------------
// Public classes and functions

namespace skytether {

  KBuffer::~KBuffer() { Release(); }
  KBuffer::KBuffer(): base(nullptr), len(0) {}
  KBuffer::KBuffer(uint8_t *buf_base, size_t buf_len): base(buf_base), len(buf_len) {}

  // TODO: double check this is correct
  void   KBuffer::Release()  { base = nullptr; }
  string KBuffer::ToString() { return string { (char *) base, len }; }

  bool KBuffer::Matches(char *other) {
    if (other == nullptr) { return false; }
    return 0 == strncmp((char *) base, other, len);
  }

  bool KBuffer::Matches(const string& token) {
    string tmp_str { (char *) base, len };
    return token.compare(tmp_str) == 0;
  }

  bool KBuffer::Matches(KBuffer *other) {
    if (other == nullptr or len != other->len) { return false; }
    return 0 == strncmp((char *) base, (char *) other->base, len);
  }

  Result<shared_ptr<RecordBatchStreamReader>>
  KBuffer::NewRecordBatchReader() {
    return RecordBatchStreamReader::Open(
       std::make_shared<BufferReader>(base, (int64_t) len)
      ,IPCReadOpts::Defaults()
    );
  }

} // namespace: skytether
