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
// Substrait is a specification of how to represent a query plan. Mohair is a protocol
// that extends substrait in order to propagate query plans between distributed query
// engines for cooperative query decomposition. Each query engine that uses the mohair
// protocol should cooperate on query processing and execution.


// ------------------------------
// Dependencies
#pragma once

// >> Definitions for mohair protocol
#include "mohair/api.hpp"
#include "skyproto/substrait/type.pb.h"


// ------------------------------
// Type Aliases

namespace skytether {

  //  >> Protobuf framework types
  using google::protobuf::TextFormat;
  using AnyMessage = google::protobuf::Any;

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

  using mohair::SystemPlan;

  // TODO: see if these should be aliased in mohair
  using SubstraitType       = skyproto::substrait::Type;
  using SubstraitExpression = skyproto::substrait::Expression;
  using SubstraitSortField  = skyproto::substrait::SortField;

} // namespace: skytether

