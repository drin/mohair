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

#include "skytether.hpp"
#include "mohair/api.hpp"

#include <google/protobuf/text_format.h>


// ------------------------------
// Aliases

// >> Namespace Aliases
namespace fs = std::filesystem;

// >> Type Aliases
using std::string;
using std::unique_ptr;
using std::vector;

using mohair::SubstraitMessage;
using mohair::SystemPlan;
using mohair::PipelineStage;
using mohair::PlanSplit;


// ------------------------------
// Functions
int ValidateArgs(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: mohair <path-to-substrait-plan>" << std::endl;
    return 1;
  }

  return 0;
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
  int validate_status = ValidateArgs(argc, argv);
  if (validate_status != 0) {
    std::cerr << "Failed to validate input command-line args" << std::endl;
    return validate_status;
  }

  // Process CLI arg a bit
  string substrait_fpath { argv[1] };
  string substrait_fname { fs::path(substrait_fpath).stem() };

  // Read the example substrait from a file
  auto substrait_msg = SubstraitMessage::FromFile(substrait_fpath);
  if (substrait_msg->payload == nullptr) {
    std::cerr << "Failed to read substrait plan from file" << std::endl;
    return 2;
  }

  // Convert substrait to a plan we understand
  // NOTE: keep this alive, everything else references from it.
  std::cout << "Parsing Substrait..." << std::endl;
  unique_ptr<SystemPlan> sys_plan { mohair::SystemPlanFrom(std::move(substrait_msg)) };

  if (sys_plan == nullptr) {
    std::cerr << "Failed to construct system plan" << std::endl;
    return 10;
  }

  sys_plan->PrintPipelines();

  // >> split super-plan into sub-plans
  int    msg_count = 1;
  size_t count_splits { sys_plan->pipeline_stages.size() };

  for (size_t stage_ndx = 0; stage_ndx < count_splits; ++stage_ndx) {
    PipelineStage* plan_stage  { sys_plan->pipeline_stages[stage_ndx].get() };
    PlanSplit      stage_split { plan_stage };

    const auto& subplan_msgs = stage_split.SubplansFor(sys_plan->plan_msg.get());
    for (size_t msg_ndx = 0; msg_ndx < subplan_msgs.size(); ++msg_ndx) {
      string out_fname {
        "resources/subplans/" +       substrait_fname
                              + "." + std::to_string(stage_ndx)
                              + "." + std::to_string(msg_ndx)
                              + "." + std::to_string(msg_count++)
                              + ".substrait"
      };

      std::cout << "\tWriting to file [" << out_fname << "]" << std::endl;

      SubstraitMessage& msg = dynamic_cast<SubstraitMessage&>(*(subplan_msgs[msg_ndx]));
      auto success = msg.SerializeToFile(out_fname.data());

      if (not success) {
        std::cerr << "\tError during serialization" << std::endl;
        return 12;
      }
    }
  }

  return 0;
}
