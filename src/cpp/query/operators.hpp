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

namespace skytether {

  // >> Standard types
  using std::tuple;
  using std::get;
  using std::get_if;

  // >> Substrait types
  using skyproto::substrait::ProjectRel;
  using skyproto::substrait::FilterRel;
  using skyproto::substrait::FetchRel;

  using skyproto::substrait::SortRel;
  using skyproto::substrait::AggregateRel;

  using skyproto::substrait::CrossRel;
  using skyproto::substrait::JoinRel;

  using skyproto::substrait::HashJoinRel;
  using skyproto::substrait::MergeJoinRel;

  using skyproto::substrait::ReadRel;

  using skyproto::substrait::ExtensionLeafRel;
  using skyproto::mohair::SkyRel;
  using skyproto::mohair::SkyPartitionRel;
  using skyproto::mohair::SkySliceRel;

  // >> Convenience aliases
  using LocalFiles       = skyproto::substrait::ReadRel::LocalFiles;
  using FileOrFiles      = skyproto::substrait::ReadRel::LocalFiles::FileOrFiles;
  using ArrowReadOptions = skyproto::substrait::ReadRel::LocalFiles::FileOrFiles::ArrowReadOptions;

} // namespace: skytether


// ------------------------------
// Classes and Methods

namespace skytether {

  // ------------------------------
  // Operators

  // >> Leaf operators
  struct OpErr : QueryOp {
    string err_msg;

    OpErr(Rel *rel, const char *msg): QueryOp(rel), err_msg(msg) {}

    const string ToString() override;
  };

  struct OpRead : PipelineOp {
    ReadRel *plan_op;

    OpRead(ReadRel *op, Rel *rel, string &tname)
      : PipelineOp(rel, tname), plan_op(op) {}

    const string ToString() override;
  };

  //! An operator that represents an extension operator holding a `mohair::SkyRel`.
  struct OpSkyRead : PipelineOp {
    ExtensionLeafRel*  plan_op;
    unique_ptr<SkyRel> sky_rel;

    OpSkyRead( ExtensionLeafRel*    op
              ,Rel*                 rel
              ,unique_ptr<SkyRel>&& unpacked_rel
              ,string&              tname)
      : PipelineOp(rel, tname), plan_op(op), sky_rel(std::move(unpacked_rel)) {}

    const string ToString() override;
  };

  struct OpPartitionRead : PipelineOp {
    ExtensionLeafRel*  plan_op;
    unique_ptr<SkyPartitionRel> sky_rel;

    OpPartitionRead( ExtensionLeafRel*             op
                    ,Rel*                          rel
                    ,unique_ptr<SkyPartitionRel>&& unpacked_rel
                    ,string&                       tname)
      : PipelineOp(rel, tname), plan_op(op), sky_rel(std::move(unpacked_rel)) {}

    const string ToString() override;
  };

  struct OpSliceRead : PipelineOp {
    ExtensionLeafRel*  plan_op;
    unique_ptr<SkySliceRel> sky_rel;

    OpSliceRead( ExtensionLeafRel*         op
                ,Rel*                      rel
                ,unique_ptr<SkySliceRel>&& unpacked_rel
                ,string&                   tname)
      : PipelineOp(rel, tname), plan_op(op), sky_rel(std::move(unpacked_rel)) {}

    const string ToString() override;
  };

  // >> Complete definitions of query operators
  //  |> Pipeline-able operators
  struct OpProj : PipelineOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    ProjectRel *plan_op;
    InputsType  op_inputs;

    OpProj(ProjectRel *op, Rel *rel, string &tname)
      : PipelineOp(rel, tname), plan_op(op) {}

    const string           ToString()     override;
    std::vector<QueryOp *> GetOpInputs()  override;
    unique_ptr<SuperPlan>  ToSuperPlanRef() override;
  };

  struct OpSel : PipelineOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    FilterRel  *plan_op;
    InputsType  op_inputs;

    OpSel(FilterRel *op, Rel *rel, string &tname)
      : PipelineOp(rel, tname), plan_op(op) {}

    const string           ToString()     override;
    std::vector<QueryOp *> GetOpInputs()  override;
    unique_ptr<SuperPlan>  ToSuperPlanRef() override;
  };

  struct OpLimit : PipelineOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    FetchRel   *plan_op;
    InputsType  op_inputs;

    OpLimit(FetchRel *op, Rel *rel, string &tname)
      : PipelineOp(rel, tname), plan_op(op) {}

    const string           ToString()     override;
    std::vector<QueryOp *> GetOpInputs()  override;
    unique_ptr<SuperPlan>  ToSuperPlanRef() override;
  };


  //  |> Pipeline breaking operators
  struct OpSort : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    SortRel    *plan_op;
    InputsType  op_inputs;

    OpSort(SortRel *op, Rel *rel, string &tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    const string           ToString()     override;
    std::vector<QueryOp *> GetOpInputs()  override;
    unique_ptr<SuperPlan>  ToSuperPlanRef() override;
  };

  struct OpAggr : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>>;

    AggregateRel *plan_op;
    InputsType    op_inputs;

    OpAggr(AggregateRel *op, Rel *rel, string &tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    const string           ToString()     override;
    std::vector<QueryOp *> GetOpInputs()  override;
    unique_ptr<SuperPlan>  ToSuperPlanRef() override;
  };

  struct OpCrossJoin : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>;

    CrossRel   *plan_op;
    InputsType  op_inputs;

    OpCrossJoin(CrossRel *op, Rel *rel, string &tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    const string           ToString()     override;
    std::vector<QueryOp *> GetOpInputs()  override;
    unique_ptr<SuperPlan>  ToSuperPlanRef() override;
  };

  struct OpJoin : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>;

    JoinRel    *plan_op;
    InputsType  op_inputs;

    OpJoin(JoinRel *op, Rel *rel, string &tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    const string           ToString()     override;
    std::vector<QueryOp *> GetOpInputs()  override;
    unique_ptr<SuperPlan>  ToSuperPlanRef() override;
  };

  struct OpHashJoin : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>;

    HashJoinRel *plan_op;
    InputsType   op_inputs;

    OpHashJoin(HashJoinRel *op, Rel *rel, string &tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    const string           ToString()     override;
    std::vector<QueryOp *> GetOpInputs()  override;
    unique_ptr<SuperPlan>  ToSuperPlanRef() override;
  };

  struct OpMergeJoin : BreakerOp {
    using InputsType = tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>;

    MergeJoinRel *plan_op;
    InputsType    op_inputs;

    OpMergeJoin(MergeJoinRel *op, Rel *rel, string &tname)
      : BreakerOp(rel, tname), plan_op(op) {}

    const string           ToString()     override;
    std::vector<QueryOp *> GetOpInputs()  override;
    unique_ptr<SuperPlan>  ToSuperPlanRef() override;
  };

  /* TODO: needs variadic op_inputs
  struct OpSet : BreakerOp {
    SetRel                          *plan_op;
    tuple<unique_ptr<QueryOp>, ...>  op_inputs;
  };
  */


  // >> Convenience functions
  string SourceNameForRead(ReadRel *substrait_op);

} // namespace: skytether
