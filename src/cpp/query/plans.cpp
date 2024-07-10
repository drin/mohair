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
  unique_ptr<QueryOp> MohairPlanFrom(PlanMessage& plan_msg) {
    // walk the top level relations until we find the root (should only be one)
    int root_ndx = FindPlanRoot(*(plan_msg.payload));

    // set the plan root if not already set
    if (plan_msg.root_relndx < 0) {
      plan_msg.root_relndx   = root_ndx;
      plan_msg.root_relation = plan_msg.payload->mutable_relations(root_ndx);
    }

    // build our internal representation from the top level `Rel`
    return MohairFrom(plan_msg.root_relation->mutable_root()->mutable_input());
  }


  // >> Query plan traversal functions
  size_t FindTallJoinLeaf(PlanVec &plans) {
    int    tallest_height = 0;
    size_t match_ndx      = 0;

    // An element in PlanVec may be null if we previously moved it
    for (size_t plan_ndx = 0; plan_ndx < plans.size(); ++plan_ndx) {
      unique_ptr<AppPlan>& plan = plans[plan_ndx];
      if (plan == nullptr) { continue; }

      // we're only interested in bottom-most join operators
      if (plan->attrs.plan_width != 2) { continue; }

      // track the index that matches our criteria
      if (plan->attrs.plan_height > tallest_height) {
        std::cout << "[" << std::to_string(plan_ndx) << "]\tHeight: "
                  <<        std::to_string(plan->attrs.plan_height)
                  << std::endl
        ;

        match_ndx      = plan_ndx;
        tallest_height = plan->attrs.plan_height;
      }
    }

    return match_ndx;
  }


  size_t FindLongPipelineLeaf(PlanVec &plans) {
    int    longest_pipelen = 0;
    size_t match_ndx       = 0;

    // An element in PlanVec may be null if we previously moved it
    for (size_t plan_ndx = 0; plan_ndx < plans.size(); ++plan_ndx) {
      unique_ptr<AppPlan>& plan = plans[plan_ndx];
      if (plan == nullptr) { continue; }

      // track the index that matches our criteria
      if (plan->attrs.pipe_len > longest_pipelen) {
        std::cout << "[" << std::to_string(plan_ndx) << "]\tLen: "
                  <<        std::to_string(plan->attrs.pipe_len)
                  << std::endl
        ;

        match_ndx       = plan_ndx;
        longest_pipelen = plan->attrs.pipe_len;
      }
    }

    return match_ndx;
  }


  unique_ptr<PlanSplit>
  DecomposePlan(AppPlan& plan, DecomposeAlg method) {
    switch (method) {
      case TallJoinLeaf: {
        size_t split_ndx = FindTallJoinLeaf(plan.break_ops);
        return std::make_unique<PlanSplit>(plan, *(plan.break_ops[split_ndx]));
      }

      // LongPipelineLeaf is currently default algorithm
      case LongPipelineLeaf: {
        size_t split_ndx = FindLongPipelineLeaf(plan.bleaf_ops);
        return std::make_unique<PlanSplit>(plan, *(plan.bleaf_ops[split_ndx]));
      }

      default: {
        std::cerr << "Unknown decomposition method" << std::endl;
        return nullptr;
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
  PlanAttrs& PlanAttrs::operator=(const PlanAttrs& other) {
    pipe_len     = other.pipe_len;
    plan_width   = other.plan_width;
    plan_height  = other.plan_height;
    break_height = other.break_height;

    return *this;
  }

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

  unique_ptr<AppPlan> AppPlanFromQueryOp(QueryOp *op) {
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
   * A method that creates a substrait message for each subplan derived from a PlanSplit.
   *
   * For each subplan, the general process is to:
   *  1. create a copy of the original substrait message
   *  2. replace the original root rel with the root rel of the sub-plan
   *  3. set an anchor (rel in super-plan), which identifies the sink for the sub-plan.
   * 
   * The order of 2 and 3 is unimportant (we already have references to both in
   * PlanSplit). Step 2 is what will allow the next consumer to only see the sub-plan.
   * Step 3 will allow us to make merging of the pushback plan trivial (we will be able to
   * use operator equality).
   */
  vector<unique_ptr<SubstraitMessage>>
  SubstraitMessage::SubplansFromSplit(PlanSplit& split) {
    // Get the anchor op and initialize some variables
    QueryOp*         anchor_op     = split.anchor_op.plan_op;
    auto             anchor_msg    = PlanAnchorFrom(anchor_op);
    vector<QueryOp*> anchor_inputs = anchor_op->GetOpInputs();

    // This is a debug macro
    MohairDebugMsg(
         "Anchor op has ["
      << std::to_string(anchor_inputs.size())
      << "] inputs"
    );

    // Initialize the list of messages to return
    vector<unique_ptr<SubstraitMessage>> subplan_msgs;
    subplan_msgs.reserve(anchor_inputs.size());

    // Create a substrait message for each input to the anchor
    for (size_t input_ndx = 0; input_ndx < anchor_inputs.size(); ++input_ndx) {
      QueryOp* input_op        = anchor_inputs[input_ndx];
      Rel*     subplan_rootrel = input_op->op_wrap;

      // Create a copy of the original substrait message that we can modify
      auto subplan_msg = std::make_unique<Plan>();
      subplan_msg->CopyFrom(*(this->payload));

      // Set the `PlanAnchor` message and replace the super-plan root
      auto subplan_planext = subplan_msg->mutable_advanced_extensions();
      auto subplan_oldroot = subplan_msg->mutable_relations(this->root_relndx)->mutable_root();

      subplan_planext->mutable_optimization()->PackFrom(*(anchor_msg));
      subplan_oldroot->mutable_input()->CopyFrom(*subplan_rootrel);

      // Add a `SubstraitMessage` that wraps the `Plan` message
      subplan_msgs.push_back(
        std::make_unique<SubstraitMessage>(std::move(subplan_msg), this->root_relndx)
      );
    }

    return subplan_msgs;
  }

} // namespace: mohair
