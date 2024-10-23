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

#include "mohair.hpp"
#include "query/plans.hpp"

#include <google/protobuf/text_format.h>


// >> Type Aliases
using mohair::QueryOp;
using mohair::AppPlan;
using mohair::DecomposeAlg;
using mohair::SubstraitMessage;
using mohair::PlanSplit;

using google::protobuf::TextFormat;


// >> Function Aliases
using mohair::InputStreamForFile;
using mohair::OutputStreamForFile;
using mohair::MohairPlanFrom;
using mohair::AppPlanFromQueryOp;
using mohair::DecomposePlan;


// ------------------------------
// Functions
int ValidateArgs(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: mohair <path-to-substrait-plan>" << std::endl;
    return 1;
  }

  return 0;
}

void PrintPlans(vector<unique_ptr<AppPlan>>& plan_list) {
  for (size_t plan_ndx = 0; plan_ndx < plan_list.size(); ++plan_ndx) {
    std::cout << "\t[" << std::to_string(plan_ndx) << "]" << std::endl;

    unique_ptr<AppPlan>& app_plan = plan_list[plan_ndx];
    std::cout << app_plan->ViewPlan() << std::endl;
  }
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
  int validate_status = ValidateArgs(argc, argv);
  if (validate_status != 0) {
    std::cerr << "Failed to validate input command-line args" << std::endl;
    return validate_status;
  }

  // Read the example substrait from a file
  auto file_stream   = InputStreamForFile(argv[1]);
  auto substrait_msg = std::make_unique<SubstraitMessage>(file_stream);
  if (substrait_msg->payload == nullptr) {
    std::cerr << "Failed to read substrait plan from file" << std::endl;
    return 2;
  }

  // Convert substrait to a plan we understand
  // NOTE: keep this alive, everything else references from it.
  std::cout << "Parsing Substrait..." << std::endl;
  unique_ptr<QueryOp> mohair_root = MohairPlanFrom(*substrait_msg);

  // NOTE: each AppPlan instance is a unique_ptr
  std::cout << "Traversing Mohair plan..." << std::endl;
  auto application_plan = AppPlanFromQueryOp(mohair_root.get());
  if (application_plan == nullptr) {
    std::cerr << "Failed to parse substrait plan" << std::endl;
    return 10;
  }

  std::cout << "Mohair Plan:" << std::endl;
  std::cout << application_plan->ViewPlan() << std::endl;

  /* NOTE: this is just to peek at the result of walking the substrait plan */
  std::cout << "Breaker Leaves:" << std::endl;
  PrintPlans(application_plan->bleaf_ops);

  std::cout << "Breaker Ops:" << std::endl;
  PrintPlans(application_plan->break_ops);

  // >> split super-plan into sub-plans
  int subplan_total = 1;

  // default to internal breakers
  size_t count_anchors = application_plan->break_ops.size();
  vector<unique_ptr<AppPlan>>* plan_anchors = &(application_plan->break_ops);

  // fallback to leaf breakers (simple subplans)
  if (application_plan->break_ops.empty()) {
    count_anchors = application_plan->bleaf_ops.size();
    plan_anchors  = &(application_plan->bleaf_ops);
  }

  for (size_t split_ndx = 0; split_ndx < count_anchors; ++split_ndx) {
    PlanSplit plan_split { *application_plan, *((*plan_anchors)[split_ndx]) };

    auto subplan_msgs = substrait_msg->SubplansFromSplit(plan_split);
    for (int subplan_ndx = 0; subplan_ndx < subplan_msgs.size(); ++subplan_ndx) {
      string out_fname {
        "resources/subplans/" +       std::to_string(split_ndx)
                              + "." + std::to_string(subplan_ndx)
                              + "." + std::to_string(subplan_total++)
                              + ".substrait"
      };

      std::cout << "\tWriting to file [" << out_fname << "]" << std::endl;

      SubstraitMessage& subplan_msg = *(subplan_msgs[subplan_ndx]);
      auto success = subplan_msg.SerializeToFile(out_fname.data());

      if (not success) {
        std::cerr << "\tError during serialization" << std::endl;
        return 12;
      }
    }
  }

  return 0;
}
