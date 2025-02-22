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
#pragma once


// ------------------------------
// Classes

namespace skytether {

  struct KBuffer {
      uint8_t *base;
      size_t   len;

      ~KBuffer();
      KBuffer();
      KBuffer(uint8_t *buf_base, size_t buf_len);

      std::string ToString();

      void Release();
      bool Matches(char *other);
      bool Matches(std::string other);
      bool Matches(KBuffer *other);

      Result<shared_ptr<RecordBatchStreamReader>>
      NewRecordBatchReader();
  };

} // namespace: skytether
