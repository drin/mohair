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

  // >> Conversion functions

  unique_ptr<PlanRel> SubstraitPlanFromString(string &plan_msg) {
    auto substrait_plan = std::make_unique<PlanRel>();

    substrait_plan->ParseFromString(plan_msg);
    substrait_plan->PrintDebugString();

    return substrait_plan;
  }

  unique_ptr<PlanRel> SubstraitPlanFromBuffer(const shared_ptr<Buffer> &plan_msg) {
    return SubstraitPlanFromString(plan_msg->ToString());
  }

  unique_ptr<PlanRel> SubstraitPlanFromFile(fstream *plan_fstream) {
    auto substrait_plan = std::make_unique<PlanRel>();
    if (substrait_plan->ParseFromIstream(plan_fstream)) { return substrait_plan; }

    std::cerr << "Failed to parse substrait plan" << std::endl;
    return nullptr;
  }

  unique_ptr<QueryOp> MohairPlanFrom(unique_ptr<PlanRel> &substrait_plan) {
    switch (substrait_plan->rel_type_case()) {

      case PlanRel::RelTypeCase::kRoot: {
        // unique_ptr<PlanRel> -> &RootRel -> *Rel
        return MohairFrom(substrait_plan->root().mutable_input());
      }

      default: { std::cerr << "Expected a PlanRel with a root" << std::endl; }
    }

    return nullptr;
  }


  // >> Query plan traversal functions

} // namespace: mohair


// ------------------------------
// Classes

namespace mohair {

  // >> Helper functions

  void InsertionSortAppPlans(std::vector<AppPlan> &plans, AppPlan &new_plan) {
    int      plan_ndx { 0 };
    AppPlan *other;

    // find the spot to insert
    for (; plan_ndx < plans.size(); ++plan_ndx) {
      if (new_plan.pipeline_len > plans[plan_ndx].pipeline_len) { break; }
    }

    // we always insert, even if its at the end of the list
    plans.insert(plan_ndx, new_plan);
  }

  int CalcMaxPipelineLength(std::vector<AppPlan> &plans) {
    int pipeline_len { 0 };

    for (auto &plan : plans) {
      if (plan.plan_root->IsBreaker())      { continue; }
      if (plan.pipeline_len > pipeline_len) { pipeline_len = plan.pipeline_len; }
    }

    return pipeline_len;
  }

  int SumPlanProperties(std::vector<AppPlan> &plans, std::function<int(AppPlan&)> fn_get) {
    int prop_val { 0 };
    for (auto &plan : plans) { prop_val += fn_get(plan); }
    return prop_val;
  }

  int MaxPlanProperties(std::vector<AppPlan> &plans, std::function<int(AppPlan&)> fn_get) {
    int prop_val { 0 };

    for (auto &plan : plans) {
      int plan_val = fn_get(plan);
      if (plan_val > prop_val) { prop_val = plan_val; }
    }

    return prop_val;
  }

  template <typename StoredType>
  std::vector<StoredType>
  GatherPlanProperties( std::vector<AppPlan> &plans
                       ,std::function<std::vector<StoredType>(AppPlan&)> fn_get) {
    std::vector<StoredType> gathered_props;

    for (auto &plan : plans) {
      auto &plan_props = fn_get(plan);
      gathered_props.insert(0, plan_props.cbegin(), plan_props.cend());
    }

    return std::move(gathered_props);
  }

  // >> DecomposableProperties functions
  DecomposableProperties::DecomposableProperties( int plen, int pwidth, int pheight
                                                 ,int bheight, int bcount
                                                 ,std::vector<AppPlan*> vec_b
                                                 ,std::vector<AppPlan*> vec_bl)
    :  pipeline_len(plen)
      ,plan_width(pwidth)
      ,plan_height(pheight)
      ,breaker_height(bheight)
      ,breaker_count(bcount)
      ,breakers(vec_b)
      ,breaker_leaves(vec_bl)
  {
  }

  static DecomposableProperties
  DecomposableProperties::ForPlanInputs(std::vector<AppPlan> &child_plans) {
    // closures for property access
    auto get_planwidth     = [](AppPlan &plan) { return plan.attrs.plan_width;     }
    auto get_planheight    = [](AppPlan &plan) { return plan.attrs.plan_height;    }
    auto get_breakerheight = [](AppPlan &plan) { return plan.attrs.breaker_height; }
    auto get_breakercount  = [](AppPlan &plan) { return plan.attrs.breaker_coount; }

    auto get_breakers       =  [](AppPlan &plan) { return plan.attrs.breakers;       }
    auto get_breaker_leaves =  [](AppPlan &plan) { return plan.attrs.breaker_leaves; }

    auto get_pipelen = [](AppPlan &plan) {
      if (plan.plan_root->IsBreaker()) { return 0; }
      return plan.attrs.pipeline_len;
    }

    // calculate properties using helper functions and closures
    int pipeline_len   = 1 + MaxPlanProperties(plans, get_pipelen);
    int plan_height    = 1 + MaxPlanProperties(plans, get_planheight);

    int plan_width     = SumPlanProperties(plans, get_planwidth);
    if (plan_width == 0) { plan_width = 1; }

    // these get special updates elsewhere, based on the root of the super-plan
    int breaker_height = MaxPlanProperties(plans, get_breakerheight);
    int breaker_count  = SumPlanProperties(plans, get_breakercount);

    std::vector<AppPlan> breakers = GatherPlanProperties<AppPlan>(plans, get_breakers);
    std::vector<AppPlan> breaker_leaves = GatherPlanProperties<AppPlan>(
      plans, get_breaker_leaves
    );

    return DecomposableProperties(
       pipeline_len, plan_width, plan_height
      ,breaker_height, breaker_count
      ,breakers
      ,breaker_leaves
    );
  }

  // >> AppPlan static functions
  static AppPlan AppPlan::FromPlanMessage(const shared_ptr<Buffer> &plan_msg) {
    unique_ptr<PlanRel> substrait_plan { SubstraitPlanFromBuffer(plan_msg) };

    // MohairPlanFrom receives the unique_ptr by reference and returns a mutable pointer
    unique_ptr<QueryOp> plan_root_op   { MohairPlanFrom(substrait_plan) };

    // TraversePlan receives the unique_ptr by reference and returns an AppPlan
    auto application_plan      = AppPlan::TraversePlan(plan_root_op.get());
    application_plan.plan_root = std::move(plan_root_op);

    // TODO: figure out if we should save the parsed `substrait_plan`
    //       (probably useful once we start pushing down subplans)
    return application_plan;
  }

  static std::vector<AppPlan> AppPlan::WalkInputOps(QueryOp *parent_op) {
    // Gather child operators
    std::vector<QueryOp *> child_ops = parent_op->GetOpInputs();

    // Initialize a vector of AppPlan instances (wraps QueryOp with tree properties)
    std::vector<AppPlan> sorted_plans;
    sorted_plans.reserve(child_ops.size());

    // Recurse on first child operator; populate first element of sorted_plans
    sorted_plans.push_back(AppPlan::TraversePlan(child_ops[0]));

    // For each input operator to parent_op
    for (int child_ndx = 1; child_ndx < child_ops.size(); ++child_ndx) {
      // recurse on child operator
      AppPlan child_plan = AppPlan::TraversePlan(child_ops[child_ndx]);

      // insertion sort into sorted_plans
      InsertionSortAppPlans(sorted_plans, child_plan);
    }

    // return AppPlan instances sorted by pipeline length (height-related property)
    return std::move(sorted_plans);
  }

  static AppPlan AppPlan::TraversePlan(QueryOp *op) {
    // Recurse on input operators
    auto child_plans = AppPlan::WalkInputOps(op);

    // Create a plan that wraps this operator
    AppPlan root_plan { op };

    // Assign initial properties to root_plan
    root_plan.attrs = DecomposableProperties::ForPlanInputs(child_plans);

    // Then update properties in case this operator is a pipeline breaker
    if (op->IsBreaker()) {
      root_plan.attrs.breaker_count  += 1;
      root_plan.attrs.breaker_height += 1;

      // We're a breaker; we append to the end as we build up
      root_plan.attrs.breakers.push_back(root_plan);

      // If we don't have leaves, then we are a leaf
      if (child_plans.size() == 0) {
        root_plan.attrs.breaker_leaves.push_back(root_plan);
      }
    }

    // Done with traversal
    return root_plan;
  }

} // namespace: mohair
