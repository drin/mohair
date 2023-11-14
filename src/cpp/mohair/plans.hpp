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
#include "mohair.hpp"

//    |> generated protobuf code
#include "substrait/algebra.pb.h"
#include "substrait/plan.pb.h"

#include "mohair/algebra.pb.h"

//  >> External libs
#include <google/protobuf/text_format.h>


// ------------------------------
// Type aliases

//  >> Substrait types
using substrait::Plan;
using substrait::PlanRel;
using substrait::Rel;

using mohair::PlanAnchor;
using mohair::ErrRel;

//  >> Protobuf types
using google::protobuf::TextFormat;

// ------------------------------
// Classes and Functions

//  >> Classes and Methods
namespace mohair {

  // ------------------------------
  // Base classes for operators

  // Top-most base class
  struct QueryOp {
    Rel    *op_wrap;
    string  table_name;

    QueryOp(Rel *rel, string tname): op_wrap(rel), table_name(tname) {}

    virtual unique_ptr<PlanAnchor> plan_anchor();
    virtual string                 ToString();
    virtual const string           ViewStr();
    virtual bool                   IsBreaker();

    virtual std::vector<QueryOp *> GetOpInputs();
  };

  // Classes for distinguishing pipeline-able operators from pipeline breakers
  struct PipelineOp : QueryOp {
    PipelineOp(Rel *rel, string tname): QueryOp(rel, tname) {}

    virtual string ToString()  override;
    const   string ViewStr()   override { return u8"← " + this->ToString(); }
    bool           IsBreaker() override { return false; }
  };

  struct BreakerOp : QueryOp {
    BreakerOp(Rel *rel, string tname): QueryOp(rel, tname) {}

    virtual string ToString()  override;
    const   string ViewStr()   override { return u8"↤ " + this->ToString(); }
    bool           IsBreaker() override { return true; }
  };


  // ------------------------------
  // Base classes for query plans

  /**
   * QueryPlan is a simple wrapper around a QueryOp that is the root operator of a plan.
   *
   * `plan_root` is a non-owning pointer (for now) and QueryPlan represents a read-only
   * view onto operators of a substrait plan with various tree properties that we're
   * interested in.
   */
  struct QueryPlan {
    QueryOp *plan_op;

    QueryPlan(QueryOp *op) : plan_op(op) {};
  };


  // ------------------------------
  // Derived classes for query plans representing different levels of abstraction

  // Forward declare AppPlan (query plan with application-level intent)
  struct AppPlan;

  // Declare a struct of various tree properties that an AppPlan will have
  struct DecomposableProperties {
    int pipeline_len   = 0;
    int plan_width     = 0;
    int plan_height    = 0;
    int breaker_height = 0;
    int breaker_count  = 0;

    std::vector<AppPlan*> breakers;
    std::vector<AppPlan*> breaker_leaves;

    DecomposableProperties();
    DecomposableProperties( int plen, int pwidth, int pheight, int bheight, int bcount
                           ,std::vector<AppPlan*> vec_b
                           ,std::vector<AppPlan*> vec_bl);

    string ToString();
  };

  DecomposableProperties PropertiesForPlanInputs(std::vector<AppPlan> &child_plans);

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
    DecomposableProperties attrs;
    std::vector<string>    source_names;

    AppPlan(QueryOp *op) : QueryPlan(op) {};

    string ViewPlan();
  };

  AppPlan FromPlanOp(QueryOp *op);

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
    unique_ptr<Plan>       substrait_plan;
    std::vector<string>    source_names;
    std::vector<BreakerOp> breaker_list;

    int plan_width     { 0 };
    int plan_height    { 0 };
    int breaker_height { 0 };
    int breaker_count  { 0 };
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
  struct EnginePlan : QueryPlan {
    unique_ptr<Plan> substrait_plan;
  };


} // namespace: mohair


//  >> Functions
namespace mohair {

  // >> Reader Functions
  unique_ptr<Plan> SubstraitPlanFromString(string &plan_msg);
  unique_ptr<Plan> SubstraitPlanFromFile(fstream *plan_fstream);

  void PrintSubstraitPlan(Plan *plan_msg);
  void PrintSubstraitRel(Rel   *rel_msg);

  // >> Translation Functions
  unique_ptr<QueryOp>    MohairFrom(Rel *rel_msg);
  unique_ptr<QueryOp>    MohairPlanFrom(Plan *substrait_plan);
  unique_ptr<PlanAnchor> PlanAnchorFrom(unique_ptr<QueryOp> &mohair_op);


  // >> Functions for query plan traversal


} // namespace: mohair
