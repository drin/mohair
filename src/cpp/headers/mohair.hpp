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
#include <memory>
#include <iostream>

// >> Internal libs
//  |> generated protobuf code
#include "substrait/algebra.pb.h"

// >> Aliases
//  |> For memory types
using std::shared_ptr;
using std::unique_ptr;

using std::string;

//  |> For substrait
using substrait::Rel;
using substrait::ErrRel;
using substrait::PlanAnchor;

// TODO: this is here as a hack; it should eventually be removed
using substrait::OpJoin;


// ------------------------------
// Public classes and functions

namespace mohair {


  // ------------------------------
  // Base classes for operators

  // empty base class
  struct QueryOp {};

  // classes for distinguishing pipeline-able operators from pipeline breakers
  struct PipelineOp : QueryOp {
    virtual string ToString();
    const   string ViewStr() { return u8"← " + this->ToString(); }
  };

  struct BreakerOp : QueryOp {
    virtual string ToString();
    const   string ViewStr() { return u8"↤ " + this->ToString(); }
  };


  // ------------------------------
  // Base classes for query plans

  // empty base class
  struct QueryPlan {};

  // classes for distinguishing between query plans of different abstractions

  /**
   * A query plan that contains logical operators only.
   *
   * This is a query plan that abstracts decomposition and execution.
   */
  struct LogicalPlan : QueryPlan {};

  struct SubPlan : QueryPlan {};


  // ------------------------------
  // Translation Functions
  unique_ptr<QueryOp>    MohairFrom(Rel& rel_msg);
  Rel&                   SubstraitFrom(unique_ptr<QueryOp>& mohair_op);
  unique_ptr<PlanAnchor> PlanAnchorFrom(unique_ptr<OpJoin>& mohair_op);


} // namespace: mohair
