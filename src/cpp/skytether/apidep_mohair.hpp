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
  using mohair::Plan;
  using mohair::SuperPlan;
  using mohair::SubPlan;

  using mohair::PlanRel;
  using mohair::RelRoot;

  using mohair::Rel;
  using mohair::RelCommon;
  using mohair::ErrRel;
  using mohair::ExtensionLeafRel;
  using mohair::SkyResultRel;

  // >> Mohair topology types
  using mohair::ServiceConfig;
  using mohair::DeviceClass;

  // >> Mohair decomposition options
  using mohair::DecomposeAlg;

  // >> Types from mohair
  using mohair::PlanMessage;

  using mohair::SystemPlan;
  using mohair::PlanSplit;
  using mohair::DecomposeAlg;

  using mohair::SubstraitSchema;
  using mohair::SubstraitType;

  // TODO: see if these should be aliased in mohair
  using SubstraitExpression = skyproto::substrait::Expression;
  using SubstraitSortField  = skyproto::substrait::SortField;

} // namespace: skytether

