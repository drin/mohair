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

// >> Internal libs
#include "plans.hpp"


// ------------------------------
// Type Aliases

// >> Standard types
using std::tuple;
using std::get;
using std::get_if;

// >> Substrait types
using substrait::ProjectRel;
using substrait::FilterRel;
using substrait::FetchRel;

using substrait::SortRel;
using substrait::AggregateRel;

using substrait::CrossRel;
using substrait::JoinRel;

using substrait::HashJoinRel;
using substrait::MergeJoinRel;

using substrait::ReadRel;

using mohair::SkyRel;

// >> Internal types
using QueryOpVec = std::vector<QueryOp *>;


// ------------------------------
// Classes and Methods

namespace mohair {

  // ------------------------------
  // Operators

  // >> Leaf operators
  struct OpErr : QueryOp {
    string err_msg;

    OpErr(Rel *rel, const char *msg)
      : QueryOp(rel, string {""}), err_msg(msg) {}
  };

  struct OpRead : PipelineOp {
    ReadRel *plan_op;

    OpRead(ReadRel *op, Rel *rel, string tname)
      : PipelineOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override { return nullptr; }
    std::vector<QueryOp *> GetOpInputs() override { return {}; };
  };

  // TODO: figure out how this should bridge to skytether
  struct OpSkyRead : PipelineOp {
    SkyRel  *plan_op;

    unique_ptr<PlanAnchor> plan_anchor() override { return nullptr; }
    std::vector<QueryOp *> GetOpInputs() override { return {}; };
  };

  // >> Complete definitions of query operators
  //  |> Pipeline-able operators
  struct OpProj : PipelineOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    ProjectRel *plan_op;
    InputsType  op_inputs;

    OpProj(ProjectRel *op, Rel *rel, string tname)
      : PipelineOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override;
    std::vector<QueryOp *> GetOpInputs() override;
  };

  struct OpSel : PipelineOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    FilterRel  *plan_op;
    InputsType  op_inputs;

    OpSel(FilterRel *op, Rel *rel, string tname)
      : PipelineOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override;
    std::vector<QueryOp *> GetOpInputs() override;
  };

  struct OpLimit : PipelineOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    FetchRel   *plan_op;
    InputsType  op_inputs;

    OpLimit(FetchRel *op, Rel *rel, string tname)
      : PipelineOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override;
    std::vector<QueryOp *> GetOpInputs() override;
  };


  //  |> Pipeline breaking operators
  struct OpSort : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    SortRel    *plan_op;
    InputsType  op_inputs;

    OpSort(SortRel *op, Rel *rel, string tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override;
    std::vector<QueryOp *> GetOpInputs() override;
  };

  struct OpAggr : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    AggregateRel *plan_op;
    InputsType    op_inputs;

    OpAggr(AggregateRel *op, Rel *rel, string tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override;
    std::vector<QueryOp *> GetOpInputs() override;
  };

  struct OpCrossJoin : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>;

    CrossRel   *plan_op;
    InputsType  op_inputs;

    OpCrossJoin(CrossRel *op, Rel *rel, string tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override;
    std::vector<QueryOp *> GetOpInputs() override;
  };

  struct OpJoin : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>;

    JoinRel    *plan_op;
    InputsType  op_inputs;

    OpJoin(JoinRel *op, Rel *rel, string tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override;
    std::vector<QueryOp *> GetOpInputs() override;
  };

  struct OpHashJoin : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>;

    HashJoinRel *plan_op;
    InputsType   op_inputs;

    OpHashJoin(HashJoinRel *op, Rel *rel, string tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override;
    std::vector<QueryOp *> GetOpInputs() override;
  };

  struct OpMergeJoin : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>;

    MergeJoinRel *plan_op;
    InputsType    op_inputs;

    OpMergeJoin(MergeJoinRel *op, Rel *rel, string tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    unique_ptr<PlanAnchor> plan_anchor() override;
    std::vector<QueryOp *> GetOpInputs() override;
  };

  /* TODO: needs variadic op_inputs
  struct OpSet : BreakerOp {
    SetRel                          *plan_op;
    tuple<unique_ptr<QueryOp>, ...>  op_inputs;
  };
  */


  // >> Convenience functions
  string SourceNameForRead(ReadRel *substrait_op);

} // namespace: mohair
