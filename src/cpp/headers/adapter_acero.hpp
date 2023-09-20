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

#include <arrow/api.h>
#include <arrow/engine/api.h>

// NOTE: arrow/acero/api.h is in 13.0.0 (dev) but not 12.0.1 (stable)
// #include <arrow/acero/api.h>
#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>


// ------------------------------
// Classes and Functions

namespace mohair::adapters {

  // TODO: use the following for simple execution
  //  Result<std::shared_ptr<RecordBatchReader>> ExecuteSerializedPlan(
  //     const Buffer& substrait_buffer
  //    ,const ExtensionIdRegistry* registry
  //    ,compute::FunctionRegistry* func_registry
  //    ,const ConversionOptions& conversion_options
  //    ,bool use_threads
  //    ,MemoryPool* memory_pool
  //  )
  struct SubstraitPlan {
  };

  SubstraitPlan FromMessageBytes(const uint8_t *data, int64_t size);

} // mohair::adapters
