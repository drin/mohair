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

  // >> Debug functions

  template <typename MsgType>
  void PrintProtoMessage(MsgType *msg) {
    string proto_str;

    auto success = TextFormat::PrintToString(*msg, &proto_str);
    if (not success) {
      std::cerr << "Unable to print message" << std::endl;
      return;
    }

    std::cout << "Proto message:" << std::endl
              << proto_str        << std::endl;
  }

  void  PrintSubstraitRel(Rel  *rel_msg ) { PrintProtoMessage<Rel>(rel_msg);   }
  void PrintSubstraitPlan(Plan *plan_msg) { PrintProtoMessage<Plan>(plan_msg); }


  // >> Conversion functions
  unique_ptr<Plan> SubstraitPlanFromString(string &plan_msg) {
    auto substrait_plan = std::make_unique<Plan>();

    substrait_plan->ParseFromString(plan_msg);
    // substrait_plan->PrintDebugString();

    return substrait_plan;
  }

  unique_ptr<Plan> SubstraitPlanFromFile(fstream *plan_fstream) {
    auto substrait_plan = std::make_unique<Plan>();
    if (substrait_plan->ParseFromIstream(plan_fstream)) { return substrait_plan; }

    std::cerr << "Failed to parse substrait plan" << std::endl;
    return nullptr;
  }

  unique_ptr<QueryOp> MohairPlanFrom(Plan *substrait_plan) {
    // walk the top level relations until we find the root (should only be one)
    int root_count = 0;
    unique_ptr<QueryOp> mohair_root;

    for (int ndx = 0; ndx < substrait_plan->relations_size(); ++ndx) {
      PlanRel plan_root { substrait_plan->relations(ndx) };

      // Don't break after we find a RootRel; validate there is only 1
      if (plan_root.rel_type_case() == PlanRel::RelTypeCase::kRoot) {
        ++root_count;
        mohair_root = MohairFrom(plan_root.mutable_root()->mutable_input());
      }
    }

    if (root_count != 1) {
      std::cerr << "Found [" << std::to_string(root_count) << "] RootRels" << std::endl;
      return nullptr;
    }

    return mohair_root;
  }


  // >> Query plan traversal functions
  unique_ptr<AppPlan>& FindLongPipelineLeaf(PlanVec &plans) {
    int    longest_pipelen = 0;
    size_t match_ndx       = 0;

    for (size_t plan_ndx = 0; plan_ndx < plans.size(); ++plan_ndx) {
      unique_ptr<AppPlan>& plan = plans[plan_ndx];
      if (plan == nullptr) { continue; }

      // track the index that matches our criteria
      if (plan->attrs.pipe_len > longest_pipelen) {
        match_ndx       = plan_ndx;
        longest_pipelen = plan->attrs.pipe_len;
      }
    }

    return plans[match_ndx];
  }

  unique_ptr<DecomposedPlan> SplitPlan(unique_ptr<AppPlan> &plan, DecomposeAlg method) {
    switch (method) {
      // LongPipelineLeaf is currently default algorithm
      case LongPipelineLeaf:
      default: {
        // get a reference to the anchor operator
        unique_ptr<AppPlan>& plan_splitop = FindLongPipelineLeaf(plan->bleaf_ops);

        // return the DecomposedPlan
        return std::make_unique<DecomposedPlan>(
           std::move(plan), std::move(plan_splitop), plan_splitop->GetOpInputs()
        );
      }
    }
  }

  // >> DecomposedPlan member functions
  // TODO: do a bit of refactoring so that we can create a subplan message
  unique_ptr<Plan> DecomposedPlan::MessageForSubPlan() {
    auto sub_plan = std::make_unique<Plan>(query_plan->plan_op);
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
  PlanAttrs WalkPlanForDiscovery(QueryOp* parent_op, PlanVec& break_ops, PlanVec& bleaf_ops) {
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

    /*
    PlanAttrs p_attrs;
    p_attrs.pipe_len     = p_len     + 1;
    p_attrs.plan_height  = pl_height + 1;
    p_attrs.break_height = b_height;
    if (pl_width > 0) { p_attrs.plan_width = pl_width; }
    return p_attrs;
    */
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
