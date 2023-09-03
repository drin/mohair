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

#include "util.hpp"


// ------------------------------
// Functions

fstream StreamForFile(const char *in_fpath) {
  return fstream { in_fpath, std::ios::in | std::ios::binary };
}

unique_ptr<PlanRel> SubstraitPlanFromFile(fstream plan_fstream) {
  auto substrait_plan = std::make_unique<PlanRel>();
  if (substrait_plan->ParseFromIstream(&plan_fstream)) { return substrait_plan; }

  std::cerr << "Failed to parse substrait plan" << std::endl;
  return nullptr;
}