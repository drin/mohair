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

// >> Standard libs
#include <fstream>
#include <iostream>

// >> Type Aliases
using std::string;

//  |> For standard libs

// ------------------------------
// Functions

// TODO:  make convenience function to read a plan from a file
fstream StreamForFile(const char *in_fpath) {
  fstream in_fstream { in_fpath, std::ios::in | std::ios::binary };
  return in_fstream;
}
