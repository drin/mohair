// ------------------------------
// License
//
// Copyright 2024 Aldrin Montana
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
// Overview
//
// Mohair is a protocol that extends substrait in order to propagate query plans
// (represented using substrait) between query engines. Each query engine that uses the
// mohair protocol should cooperate on query processing and execution.


// ------------------------------
// Dependencies
#pragma once

// >> Common internal deps
#include "skytether.hpp"

// >> Definitions for mohair protocol
#include "mohair.hpp"

// >> Third-party deps
#include "query/apidep_substrait.hpp"


// ------------------------------
// Type Aliases

namespace skytether {

  // >> Mohair query processing types
  using skyproto::mohair::SuperPlan;
  using skyproto::mohair::SubPlan;
  using skyproto::mohair::ErrRel;

  // >> Mohair topology types
  using skyproto::mohair::ServiceConfig;
  using skyproto::mohair::DeviceClass;

  // >> Types from mohair
  using mohair::PlanMessage;
  using mohair::SubstraitMessage;

} // namespace: skytether

