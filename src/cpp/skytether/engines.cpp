// ------------------------------
// License(s)
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

#include "skytether/engines.hpp"


// ------------------------------
// Functions

namespace skytether::engines {

    //! Create a new string holding the substrait function name without the extension ID
    string FunctionBasename(const string& fn_name) {
      string fn_basename;

      for (auto &c : fn_name) {
        if (c == ':') { break; }
        fn_basename += c;
      }

      return fn_basename;
    }

    string
    RemapFunctionName(const EngineFunctionMap& fn_map, const string& fn_name) {
      string name { FunctionBasename(fn_name) };

      const auto& it = fn_map.find(name);
      if (it != fn_map.end()) { name = it->second; }

      return name;
    }

} // namespace: skytether::engines


// ------------------------------
// Classes

namespace skytether::engines {

  // >> Static variable initializations
  int32_t ContextMap::next_uuid = 0;

  // >> Method implementations for ContextMap
  int32_t ContextMap::RegisterContext(unique_ptr<QueryContext>&& new_context) {
    int32_t ctx_uuid   = ContextMap::next_uuid++;
    contexts[ctx_uuid] = std::move(new_context);

    return ctx_uuid;
  }

  QueryContext* ContextMap::GetContext(int32_t context_uuid) {
    const auto& map_entry = contexts.find(context_uuid);
    if (map_entry == contexts.end()) { return nullptr; }

    return map_entry->second.get();
  }


  // >> Method implementations for QueryEngine

  //! Given an ID for a query context, return the result of the previous execution
  ResultReader QueryEngine::ResultSetForContext(int32_t context_id) {
    QueryContext* ctx { context_map.GetContext(context_id) };

    if (ctx) { return ctx->result; }
    return nullptr;
  }

  //! Logic for execution must be implemented in engine-specific derived classes
  Status QueryEngine::ExecuteFromContext([[maybe_unused]] int32_t context_id) {
    throw NotImplementedError();
  }

} // namespace: skytether::engines
