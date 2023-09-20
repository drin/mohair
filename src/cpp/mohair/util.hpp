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
#pragma once

// >> Standard libs
#include <fstream>
#include <iostream>

// >> Internal libs
#include "../headers/mohair.hpp"


// >> Type Aliases
using std::string;
using std::fstream;


// ------------------------------
// Functions

fstream StreamForFile(const char *in_fpath);
bool    FileToString(const char *in_fpath, string &file_data);

unique_ptr<PlanRel> SubstraitPlanFromString(string &plan_msg);
unique_ptr<PlanRel> SubstraitPlanFromFile(fstream *plan_fstream);
