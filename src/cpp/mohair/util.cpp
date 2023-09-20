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

bool FileToString(const char *in_fpath, string &file_data) {
  // create an IO stream for the file
  auto file_stream = StreamForFile(in_fpath);
  if (!file_stream) {
    std::cerr << "Failed to open IO stream for file" << std::endl;
    return false;
  }

  // go to end of stream, read the position, then reset position
  file_stream.seekg(0, std::ios_base::end);
  auto size = file_stream.tellg();
  file_stream.seekg(0);
  std::cout << "File size: [" << std::to_string(size) << "]" << std::endl;

  // Resize the output and read the file data into it
  file_data.resize(size);
  auto output_ptr = &(file_data[0]);
  file_stream.read(output_ptr, size);

  // On success, the number of characters read will match size
  return file_stream.gcount() == size;
}

unique_ptr<PlanRel> SubstraitPlanFromString(string &plan_msg) {
  auto substrait_plan = std::make_unique<PlanRel>();

  substrait_plan->ParseFromString(plan_msg);
  substrait_plan->PrintDebugString();

  return substrait_plan;
}

unique_ptr<PlanRel> SubstraitPlanFromFile(fstream *plan_fstream) {
  auto substrait_plan = std::make_unique<PlanRel>();
  if (substrait_plan->ParseFromIstream(plan_fstream)) { return substrait_plan; }

  std::cerr << "Failed to parse substrait plan" << std::endl;
  return nullptr;
}
