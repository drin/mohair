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
#pragma once

// >> Standard libs
#include <functional>
#include <memory>

#include <string>
#include <iostream>

#include <vector>

// >> Internal libs

//  |> all arrow dependencies and aliases
#include "arrow_deps.hpp"

//  |> generated protobuf code
#include "../mohair/substrait/algebra.pb.h"
#include "../mohair/substrait/plan.pb.h"

#include "../mohair/mohair/algebra.pb.h"

// >> Aliases
//  |> For memory types
using std::shared_ptr;
using std::unique_ptr;

using std::string;
using std::vector;

//  |> For substrait
using substrait::PlanRel;
using substrait::Rel;
using mohair::PlanAnchor;
using mohair::ErrRel;


// ------------------------------
// Public classes and functions

namespace mohair {

  // ------------------------------
  // Base classes for operators

  // empty base class
  struct QueryOp {
    Rel    *op_wrap;
    string  table_name;

    QueryOp(Rel *rel, string tname): op_wrap(rel), table_name(tname) {}

    virtual unique_ptr<PlanAnchor> plan_anchor();
    virtual string                 ToString();
    virtual const string           ViewStr();
  };

  // classes for distinguishing pipeline-able operators from pipeline breakers
  struct PipelineOp : QueryOp {
    PipelineOp(Rel *rel, string tname): QueryOp(rel, tname) {}

    virtual string ToString() override;
    const   string ViewStr() { return u8"← " + this->ToString(); }
  };

  struct BreakerOp : QueryOp {
    BreakerOp(Rel *rel, string tname): QueryOp(rel, tname) {}

    virtual string ToString() override;
    const   string ViewStr() { return u8"↤ " + this->ToString(); }
  };


  // ------------------------------
  // Base classes for query plans

  // empty base class
  struct QueryPlan {};

  // classes for distinguishing between query plans of different abstractions

  /**
   * A query plan that contains logical data manipulation operators only.
   *
   * This is a query plan that is at the same abstraction level as an application and
   * knows nothing about decomposition or execution. This is the query plan that is
   * received by a computational storage system.
   */
  struct AppPlan : QueryPlan {};

  /**
   * A query plan that may contain a mix of:
   *  - logical data manipulation operators
   *  - distributed data flow operators
   *
   * This is a query plan that knows about query plan decomposition but only knows the
   * same data processing operators as an application. This is the query plan that a
   * computational storage device may pass downstream.
   *
   * NOTE: Without information, a SysPlan is identical to an AppPlan. In the limit, a
   * SysPlan will have data flow operators that represent a "best distribution" of the
   * query plan across the computational storage system.
   *
   * TODO: do some analysis on whether we can have an optimal distribution and what that
   * would mean, etc.
   */
  struct SysPlan : QueryPlan {};

  /**
   * A query plan that contains distributed data flow operators and physical data
   * manipulation operators.
   *
   * This is a query plan that is passed to a query engine or propagates information
   * between query engines. This is the query plan that a computational storage device
   * may:
   *  - pass to a local query engine
   *  - pass to an upstream device
   *
   * NOTE: Without information, an EnginePlan is identical to a portion of the SysPlan
   * with data flow operators at the root and other data manipulation operators below
   * secondary data flow operators replaced with the equivalent of Read operators. In the
   * limit, an EnginePlan is the exact physical execution plan that a particular query
   * engine would produce and execute.
   */
  struct EnginePlan : QueryPlan {};

  // >> Translation Functions
  unique_ptr<QueryOp>    MohairFrom(Rel *rel_msg);
  unique_ptr<PlanAnchor> PlanAnchorFrom(unique_ptr<QueryOp> &mohair_op);

} // namespace: mohair
