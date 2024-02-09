// ------------------------------
// License
//
// Copyright 2024 Aldrin Montana
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

#include "messages.hpp"


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


  // >> Conversion functions (into/out of substrait plans)
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


  // >> Helper functions
  int FindPlanRoot(Plan *substrait_plan) {
    int root_count = 0;
    int root_ndx   = -1;

    for (int ndx = 0; ndx < substrait_plan->relations_size(); ++ndx) {
      PlanRel plan_root { substrait_plan->relations(ndx) };

      // Don't break after we find a RelRoot; validate there is only 1
      // We expect there to be a reasonably small number of RelRoot
      if (plan_root.rel_type_case() == PlanRel::RelTypeCase::kRoot) {
        ++root_count;
        root_ndx = ndx;
      }
    }

    if (root_count != 1) {
      std::cerr << "Found [" << std::to_string(root_count) << "] RootRels" << std::endl;
      return -1;
    }

    return root_ndx;
  }



} // namespace: mohair


// ------------------------------
// Classes

namespace mohair {

  string SubstraitMessage::Serialize() {
    string msg_serialized;

    if (not this->payload->SerializeToString(&msg_serialized)) {
      std::cerr << "Error when serializing substrait message." << std::endl;
    }

    return msg_serialized;
  }

} // namespace: mohair
