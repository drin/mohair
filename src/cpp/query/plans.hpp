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

//  >> Internal libs
#include "adapter_mohair.hpp"

#include "messages.hpp"


// ------------------------------
// Type aliases


// ------------------------------
// Base classes for query operators

namespace mohair {

  // Top-most base class
  struct QueryOp {
    Rel    *op_wrap;
    string  table_name;

    virtual ~QueryOp() {}

    QueryOp(Rel *rel)               : op_wrap(rel), table_name("")    {}
    QueryOp(Rel *rel, string &tname): op_wrap(rel), table_name(tname) {}

    virtual const string           ToString()     { return table_name;       }
    virtual const string           ViewStr()      { return this->ToString(); }
    virtual bool                   IsBreaker()    { return false;            }
    virtual vector<QueryOp *>      GetOpInputs()  { return {};               }
    virtual unique_ptr<PlanAnchor> ToPlanAnchor() { return nullptr;          }
  };

  // Classes for distinguishing pipeline-able operators from pipeline breakers
  struct PipelineOp : QueryOp {
    PipelineOp(Rel *rel, string &tname): QueryOp(rel, tname) {}

    const string ViewStr()   override { return u8"← " + this->ToString(); }
    bool         IsBreaker() override { return false; }
  };

  struct BreakerOp : QueryOp {
    BreakerOp(Rel *rel, string &tname): QueryOp(rel, tname) {}

    const string ViewStr()   override { return u8"↤ " + this->ToString(); }
    bool         IsBreaker() override { return true; }
  };

} // namespace: mohair


// ------------------------------
// Base classes for query planning and processing

namespace mohair {

  /**
   * A simple class, representing a query plan, that wraps a QueryOp (root operator).
   */
  struct QueryPlan {
    QueryOp *plan_op;

    virtual ~QueryPlan() {}
    QueryPlan(QueryOp *op) : plan_op(op) {}
  };


  // Derived classes for query plans representing different levels of abstraction

  // Forward declarations
  struct AppPlan;


  // Declare a struct of various tree properties that an AppPlan will have
  struct PlanAttrs {
    int pipe_len;
    int plan_width;
    int plan_height;
    int break_height;

    PlanAttrs(int plen, int pwidth, int pheight, int bheight)
      : pipe_len(plen), plan_width(pwidth), plan_height(pheight), break_height(bheight)
      {}

    PlanAttrs() : PlanAttrs(1, 1, 1, 0) {}
    PlanAttrs(PlanAttrs &other)
      : PlanAttrs(other.pipe_len, other.plan_width, other.plan_height, other.break_height)
      {}

    PlanAttrs& operator=(const PlanAttrs& other);

    string ToString();
  };

  /**
   * A query plan that contains logical data manipulation operators only.
   *
   * This is a query plan that is at the same abstraction level as an application and
   * knows nothing about decomposition or execution. This is the query plan that is
   * received by a computational storage system.
   *
   * The properties of an AppPlan are used for:
   *  - splitting a query into a super-plan and many sub-plans
   *  - merging a sub-plan into a super-plan (repeated to merge many sub-plans)
   */
  struct AppPlan : QueryPlan {
    // A set of attributes that a node in an AppPlan has
    PlanAttrs      attrs;
    vector<string> source_names;

    // Indices into interesting operators in the AppPlan
    vector<unique_ptr<AppPlan>> break_ops;
    vector<unique_ptr<AppPlan>> bleaf_ops;

    AppPlan(QueryOp *op)                       : QueryPlan(op)                   {};
    AppPlan(QueryOp *op, PlanAttrs &new_attrs) : QueryPlan(op), attrs(new_attrs) {};

    string ViewPlan();
  };

  using PlanVec = vector<unique_ptr<AppPlan>>;

  unique_ptr<AppPlan> AppPlanFromQueryOp(QueryOp *op);


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
   *
   * TODO: this is next step. I thought I was splitting plans in C++ already but I was
   * not. First thing to do is just do query execution, but find source names so that I
   * can access them from the kelpie pool.
   */
  struct SysPlan : QueryPlan {
    unique_ptr<PlanMessage> substrait_plan;
    vector<string>          source_names;
  };

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


} // namespace: mohair


// ------------------------------
// Classes for query processing

namespace mohair {

  /**
   * A class that points to a super-plan and an anchor operator.
   *
   * The anchor is an operator whose input is on the cut of the plan. This means that the
   * anchor is a leaf in the super-plan and a parent of each sub-plan root.
   */
  struct PlanSplit {
    AppPlan& query_plan;
    AppPlan& anchor_op;

    PlanSplit(AppPlan& qplan, AppPlan& anchor): query_plan(qplan), anchor_op(anchor) {}
  };

  enum DecomposeAlg {
     LongPipelineLeaf // Leaf pipeline breaker with longest pipeline
    ,LongPipelineHead // Internal pipeline breaker with longest pipeline
    ,TallJoinLeaf     // Leaf join operation with tallest plan height
    ,WideJoinHead     // Internal join operation with largest plan width
  };

} // namespace: mohair


// ------------------------------
// Static functions that provide convenient interfaces

namespace mohair {

  // >> Translation Functions
  unique_ptr<QueryOp>    MohairFrom(Rel *rel_msg);
  unique_ptr<QueryOp>    MohairPlanFrom(PlanMessage& substrait_plan);
  unique_ptr<PlanAnchor> PlanAnchorFrom(QueryOp* mohair_op);
  Rel&                   SubstraitRelFrom(QueryOp* mohair_op);

  // >> Functions for query plan processing
  unique_ptr<PlanSplit>
  DecomposePlan(AppPlan& plan, DecomposeAlg method = LongPipelineLeaf);

} // namespace: mohair
