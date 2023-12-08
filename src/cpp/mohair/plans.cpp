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
      if (new_plan->attrs->pipeline_len > (*plans_it)->attrs->pipeline_len) { break; }
    }

    // we always insert, even if its at the end of the list
    plans.insert(plans_it, std::move(new_plan));
  }

  // >> DecomposableProperties functions
  string DecomposableProperties::ToString() {
    std::stringstream prop_stream;

    prop_stream << "Plan properties:"   << std::endl
                << "\tPipeline length:" << pipeline_len   << std::endl
                << "\tPlan width     :" << plan_width     << std::endl
                << "\tPlan height    :" << plan_height    << std::endl
                << "\tBreaker height :" << breaker_height << std::endl
    ;

    return prop_stream.str();
  }

  // PlanVec is a vector of unique_ptr<AppPlan>
  unique_ptr<DecomposableProperties> PropertiesForPlanInputs(PlanVec &plans) {
    auto properties = std::make_unique<DecomposableProperties>();

    // pipeline length, plan width and height, and breaker height
    int p_len = 0, pl_width = 0, pl_height = 0, bheight = 0;

    // local references to vectors
    std::vector<AppPlan*> *break_ops = &(properties->breakers);
    std::vector<AppPlan*> *bleaf_ops = &(properties->breaker_leaves);

    for (size_t ndx = 0; ndx < plans.size(); ++ndx) {
      // >> grab a convenient reference to each plan; these are non-owning
      AppPlan *plan = plans[ndx].get();

      // >> pull in properties from the current plan input
      std::vector<AppPlan*> *plan_breakops = &(plan->attrs->breakers);
      std::vector<AppPlan*> *plan_bleafops = &(plan->attrs->breaker_leaves);

      int plan_plwidth  = plan->attrs->plan_width;
      int plan_plheight = plan->attrs->plan_height;
      int plan_bheight  = plan->attrs->breaker_height;

      int plan_plen = 0;
      if (not plan->plan_op->IsBreaker()) { plan_plen = plan->attrs->pipeline_len; }

      // >> update local variables
      //   |> pointers for fast access
      //      (put this first because it relates to the above check)
      else if (plan->plan_op->IsBreaker()) {
        // A breaker with no leaves must be a leaf itself
        if (plan->attrs->breaker_leaves.size() == 0) {
          bleaf_ops->push_back(plan);
          // bleaf_ops->push_back(std::move(plans[ndx]));
        }

        // Otherwise, it is an internal breaker and we preserve its pointers
        else {
          bleaf_ops->insert(bleaf_ops->cend(), plan_bleafops->cbegin(), plan_bleafops->cend());
          break_ops->insert(break_ops->cend(), plan_breakops->cbegin(), plan_breakops->cend());
          break_ops->push_back(plan);
          // break_ops->push_back(std::move(plans[ndx]));
        }
      }

      //   |> max-type properties
      if (plan_plen     > p_len    ) { p_len     = plan_plen;     }
      if (plan_plheight > pl_height) { pl_height = plan_plheight; }
      if (plan_bheight  > bheight)   { bheight   = plan_bheight;  }

      //   |> sum-type properties
      pl_width += plan_plwidth;
    }

    // >> non-conditional adjustments to properties
    ++p_len;
    ++pl_height;

    // >> conditional adjustments to properties
    if (pl_width == 0) { pl_width = 1; }

    // >> update properties container and return it
    properties->pipeline_len   = p_len;
    properties->plan_height    = pl_height;
    properties->plan_width     = pl_width;
    properties->breaker_height = bheight;

    return properties;
  }

  // >> AppPlan static functions
  std::vector<unique_ptr<AppPlan>> WalkInputOps(QueryOp *parent_op) {
    // Gather child operators
    std::vector<QueryOp *> child_ops = parent_op->GetOpInputs();

    // Initialize a vector of AppPlan instances (wraps QueryOp with tree properties)
    std::vector<unique_ptr<AppPlan>> sorted_plans;
    sorted_plans.reserve(child_ops.size());

    if (child_ops.size() == 0) { return sorted_plans; }

    // Recurse on first child operator; populate first element of sorted_plans
    auto first_childplan = FromPlanOp(child_ops[0]);
    sorted_plans.push_back(std::move(first_childplan));

    // For each input operator to parent_op
    for (size_t child_ndx = 1; child_ndx < child_ops.size(); ++child_ndx) {
      // recurse on child operator
      auto child_plan = FromPlanOp(child_ops[child_ndx]);

      // insertion sort into sorted_plans
      InsertionSortAppPlans(sorted_plans, std::move(child_plan));
    }

    // return AppPlan instances sorted by pipeline length (height-related property)
    return sorted_plans;
  }

  unique_ptr<AppPlan> FromPlanOp(QueryOp *op) {
    // Recurse on input operators
    auto child_plans = WalkInputOps(op);

    /*
    if (   child_plans.size() == 2 && child_plans[0]->plan_op->IsBreaker() && child_plans[0]->plan_op->IsBreaker()) {
      std::cout << "First case of error" << std::endl;
    }
    */

    // Create a plan that wraps this operator
    auto root_plan = std::make_unique<AppPlan>(op);

    // Assign initial properties to root_plan
    root_plan->attrs = PropertiesForPlanInputs(child_plans);

    // Update properties if this operator is a pipeline breaker
    if (op->IsBreaker()) { root_plan->attrs->breaker_height += 1; }

    // Done with traversal
    // NOTE: it seems like this gets called multiple times?
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
