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

// >> Standard libs
#include <functional>
#include <iostream>
#include <sstream>

// >> Type Aliases

//  |> For standard libs
using std::unique_ptr;
using std::string;

using std::tuple;
using std::get;

//  |> For substrait
using substrait::Rel;

using substrait::ReadRel;
using substrait::SkyRel;

using substrait::ProjectRel;
using substrait::FilterRel;
using substrait::FetchRel;

using substrait::SortRel;
using substrait::AggregateRel;

using substrait::CrossRel;
using substrait::JoinRel;

using substrait::HashJoinRel;
using substrait::MergeJoinRel;


// ------------------------------
// Functions

namespace mohair {

  // ------------------------------
  // Base classes for operators

  // empty base class
  class QueryOp {};

  // classes for distinguishing pipeline-able operators from pipeline breakers
  class PipelineOp : QueryOp {
    virtual string ToString();
    const   string ViewStr() { return u8"← " + this->ToString(); }
  };

  class BreakerOp : QueryOp {
    virtual string ToString();
    const   string ViewStr() { return u8"↤ " + this->ToString(); }
  };


  // ------------------------------
  // Operator Classes

  // >> Leaf operators
  struct OpErr : QueryOp {
    string err_msg;
    Rel    plan_op;
  };

  struct OpRead : PipelineOp {
    ReadRel            &plan_op;
    Rel                &op_wrap;
    tuple<>             op_inputs;
    unique_ptr<string>  table_name;
  };

  // TODO: figure out how this should bridge to skytether
  struct OpSkyRead : PipelineOp {
    SkyRel             &plan_op;
    Rel                &op_wrap;
    tuple<>             op_inputs;
    unique_ptr<string>  table_name;
  };

  // >> Pipeline-able operators
  struct OpProj : PipelineOp {
    ProjectRel                 &plan_op;
    Rel                        &op_wrap;
    tuple<unique_ptr<QueryOp>>  op_inputs;
    unique_ptr<string>          table_name;
  };

  struct OpSel : PipelineOp {
    FilterRel                  &plan_op;
    Rel                        &op_wrap;
    tuple<unique_ptr<QueryOp>>  op_inputs;
    unique_ptr<string>          table_name;
  };

  struct OpLimit : PipelineOp {
    FetchRel                   &plan_op;
    Rel                        &op_wrap;
    tuple<unique_ptr<QueryOp>>  op_inputs;
    unique_ptr<string>          table_name;
  };


  // >> Pipeline breaking operators
  struct OpSort : BreakerOp {
    SortRel                    &plan_op;
    Rel                        &op_wrap;
    tuple<unique_ptr<QueryOp>>  op_inputs;
    unique_ptr<string>          table_name;
  };

  struct OpAggr : BreakerOp {
    AggregateRel               &plan_op;
    Rel                        &op_wrap;
    tuple<unique_ptr<QueryOp>>  op_inputs;
    unique_ptr<string>          table_name;
  };

  struct OpCrossJoin : BreakerOp {
    CrossRel                                        &plan_op;
    Rel                                             &op_wrap;
    tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>  op_inputs;
    unique_ptr<string>                               table_name;
  };

  struct OpJoin : BreakerOp {
    JoinRel                                         &plan_op;
    Rel                                             &op_wrap;
    tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>  op_inputs;
    unique_ptr<string>                               table_name;
  };

  struct OpHashJoin : BreakerOp {
    HashJoinRel                                     &plan_op;
    Rel                                             &op_wrap;
    tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>  op_inputs;
    unique_ptr<string>                               table_name;
  };

  struct OpMergeJoin : BreakerOp {
    MergeJoinRel                                    &plan_op;
    Rel                                             &op_wrap;
    tuple<unique_ptr<QueryOp>, unique_ptr<QueryOp>>  op_inputs;
    unique_ptr<string>                               table_name;
  };

  /* TODO: needs variadic op_inputs
  struct OpSet : BreakerOp {
    SetRel                          &plan_op;
    Rel                             &op_wrap;
    tuple<unique_ptr<QueryOp>, ...>  op_inputs;
    unique_ptr<string>               table_name;
  };
  */


  // >> Translation functions
  unique_ptr<QueryOp>    MohairFrom(Rel& rel_msg);
  Rel&                   SubstraitFrom(unique_ptr<QueryOp>& mohair_op);
  unique_ptr<PlanAnchor> PlanAnchorFrom(unique_ptr<OpJoin>& mohair_op);


  // >> Convenience functions
  string SourceNameForRead(ReadRel& substrait_op);


} // namespace: mohair
