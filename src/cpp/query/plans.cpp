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

#include "plans.hpp"


// ------------------------------
// Functions

namespace mohair {

  // >> Conversion functions (into/out of mohair representation)
  unique_ptr<QueryOp> MohairPlanFrom(Plan *substrait_plan) {
    // walk the top level relations until we find the root (should only be one)
    int root_ndx = FindPlanRoot(substrait_plan);
    PlanRel plan_root { substrait_plan->relations(root_ndx) };

    return MohairFrom(plan_root.mutable_root()->mutable_input());
  }


  // >> Query plan traversal functions
  size_t FindLongPipelineLeaf(PlanVec &plans) {
    int    longest_pipelen = 0;
    size_t match_ndx       = 0;

    // An element in PlanVec may be null if we previously moved it
    for (size_t plan_ndx = 0; plan_ndx < plans.size(); ++plan_ndx) {
      unique_ptr<AppPlan>& plan = plans[plan_ndx];
      if (plan == nullptr) { continue; }

      // track the index that matches our criteria
      if (plan->attrs.pipe_len > longest_pipelen) {
        match_ndx       = plan_ndx;
        longest_pipelen = plan->attrs.pipe_len;
      }
    }

    return match_ndx;
  }

  unique_ptr<PlanSplit> DecomposePlan(unique_ptr<AppPlan> plan, DecomposeAlg method) {
    switch (method) {
      // LongPipelineLeaf is currently default algorithm
      case LongPipelineLeaf:
      default: {
        // Find which operator to use as an anchor
        size_t split_ndx = FindLongPipelineLeaf(plan->bleaf_ops);

        // return a PlanSplit (a cut in the provided query plan graph)
        return std::make_unique<PlanSplit>(
          std::move(plan), std::move(plan->bleaf_ops[split_ndx])
        );
      }
    }
  }


} // namespace: mohair


// ------------------------------
// Classes

namespace mohair {

  // >> Helper functions

  void InsertionSortAppPlans( std::vector<unique_ptr<AppPlan>> &plans
                             ,unique_ptr<AppPlan>               new_plan) {
    auto plans_it = plans.begin();

    // find the spot to insert
    for (; plans_it != plans.end(); plans_it = std::next(plans_it)) {
      if (new_plan->attrs.pipe_len > (*plans_it)->attrs.pipe_len) { break; }
    }

    // we always insert, even if its at the end of the list
    plans.insert(plans_it, std::move(new_plan));
  }

  // >> PlanAttrs functions
  string PlanAttrs::ToString() {
    std::stringstream prop_stream;

    prop_stream << "Plan properties:"   << std::endl
                << "\tPipeline length:" << pipe_len     << std::endl
                << "\tPlan width     :" << plan_width   << std::endl
                << "\tPlan height    :" << plan_height  << std::endl
                << "\tBreaker height :" << break_height << std::endl
    ;

    return prop_stream.str();
  }


  // >> AppPlan static functions

  /**
   * Walks a QueryOp DAG, propagating attributes from the leaves upwards in a depth-first
   * order. When a pipeline breaking operation is encountered (e.g. Join or Aggregate),
   * then the corresponding PlanAttrs is moved into an AppPlan and inserted into either
   * break_ops or bleaf_ops.
   */
  PlanAttrs WalkPlanForDiscovery( QueryOp* parent_op
                                 ,PlanVec& break_ops
                                 ,PlanVec& bleaf_ops) {
    // Base case: if there are no input ops, then return an empty PlanAttrs
    auto child_ops = parent_op->GetOpInputs();
    if (child_ops.size() == 0) { return PlanAttrs{}; }

    // Variables to propagate attributes without a PlanAttrs instance
    int p_len = 0, pl_width = 0, pl_height = 0, b_height = 0;
    for (size_t op_ndx = 0; op_ndx < child_ops.size(); ++op_ndx) {
      // Recurse on this op, then we can update properties
      auto child_op     = child_ops[op_ndx];
      auto child_attrs  = WalkPlanForDiscovery(child_op, break_ops, bleaf_ops);

      // Update local variables
      b_height   = std::max(b_height , child_attrs.break_height);
      pl_height  = std::max(pl_height, child_attrs.plan_height );
      pl_width  += child_attrs.plan_width;

      if (not child_op->IsBreaker()) { p_len = std::max(p_len, child_attrs.pipe_len); }

      // Update indices to breakers
      else if (child_op->IsBreaker()) {
        auto is_bleaf   = child_attrs.break_height == 1;
        auto child_plan = std::make_unique<AppPlan>(child_op, child_attrs);
        if (is_bleaf) { InsertionSortAppPlans(bleaf_ops, std::move(child_plan)); }
        else          { InsertionSortAppPlans(break_ops, std::move(child_plan)); }
      }
    }

    // Return attribute struct (with some value adjustments)
    if (parent_op->IsBreaker()) { ++b_height; }
    return PlanAttrs { p_len + 1, pl_width, pl_height + 1, b_height };
  }

  unique_ptr<AppPlan> FromPlanOp(QueryOp *op) {
    // Create a plan that wraps this operator
    auto root_plan = std::make_unique<AppPlan>(op);

    // Recurse on input operators
    root_plan->attrs = WalkPlanForDiscovery(
      op, root_plan->break_ops, root_plan->bleaf_ops
    );

    return root_plan;
  }

  void ViewOp(QueryOp *op, string *indent, std::stringstream *view_stream) {
    // Add indentation for current op
    if (op->IsBreaker()) { (*view_stream) << std::endl << *indent; }
    else                 { (*view_stream)              <<    "  "; }

    // Append representation for this op
    (*view_stream) << op->ViewStr();

    // Increase indent for child ops
    indent->append(2, ' ');

    // Recurse on child ops
    std::vector<QueryOp *> child_ops = op->GetOpInputs();
    for (size_t ndx = 0; ndx < child_ops.size(); ++ndx) {
      ViewOp(child_ops[ndx], indent, view_stream);
    }
  }

  // >> AppPlan member functions
  string AppPlan::ViewPlan() {
    std::stringstream plan_str;
    string            indent { "" };

    ViewOp(plan_op, &indent, &plan_str);

    return plan_str.str();
  }

} // namespace: mohair


// >> Implementations for SubstraitMessage that require dependencies from plans.hpp
namespace mohair {

  /**
   * A method that creates a substrait message for the pushdown plan given a PlanSplit.
   *
   * The overall process is to:
   *  1. create a copy of the original substrait message
   *  2. replace the original plan root with the new plan root (root of the sub-plan)
   *  3. set an anchor, which is the parent operator of the new plan root in the
   *     super-plan.
   * 
   * The order of 2 and 3 is unimportant (we already have references to both in
   * PlanSplit). Step 2 is what will allow the next consumer to only see the sub-plan.
   * Step 3 will allow us to make merging of the pushback plan trivial (we will be able to
   * use operator equality).
   */
  unique_ptr<SubstraitMessage> SubstraitMessage::FromSplit(PlanSplit* split) {
    // 1. Create a copy of the query plan
    auto sub_plan = std::make_unique<Plan>(*(this->payload)); 

    // grab a reference to the RelRoot
    int     root_ndx = FindPlanRoot(sub_plan.get());
    PlanRel root_rel { sub_plan->relations(root_ndx) };

    // grab a reference to the new root (TODO: make a message for each sub-plan)
    QueryOp* anchor_op    = split->anchor_op->plan_op;
    QueryOp* subplan_root = anchor_op->GetOpInputs()[0];

    // 2. Swap old root (RelRoot input) to the new root (from PlanSplit)
    // TODO: need to modify names; also need to check mem mgmt
    root_rel.mutable_root()->set_allocated_input(subplan_root->op_wrap);

    // 3. Create a PlanAnchor and pack it into the Plan
    unique_ptr<PlanAnchor> anchor_msg { anchor_op->plan_anchor() };
    sub_plan->mutable_advanced_extensions()->mutable_optimization()->PackFrom(
      *(anchor_msg.release())
    );

    return std::make_unique<SubstraitMessage>(std::move(sub_plan));
  }

} // namespace: mohair
