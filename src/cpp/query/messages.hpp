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

//  >> Internal libs
#include "../mohair.hpp"

//    |> generated protobuf code
#include "substrait/algebra.pb.h"
#include "substrait/plan.pb.h"

#include "mohair/algebra.pb.h"

//  >> External libs
#include <google/protobuf/text_format.h>


// ------------------------------
// Type Aliases

//  >> Substrait types
using substrait::Plan;
using substrait::PlanRel;
using substrait::Rel;

// >> Mohair types (substrait extension)
using mohair::PlanAnchor;
using mohair::ErrRel;

//  >> Protobuf types
using google::protobuf::TextFormat;


// ------------------------------
// Functions

namespace mohair {

  // >> Debug functions
  void PrintSubstraitPlan(Plan *plan_msg);
  void PrintSubstraitRel(Rel   *rel_msg);

  // >> Reader functions
  unique_ptr<Plan> SubstraitPlanFromString(string &plan_msg);
  unique_ptr<Plan> SubstraitPlanFromFile(fstream *plan_fstream);

  // >> Helper functions
  int FindPlanRoot(Plan& substrait_plan);

} // namespace: mohair


// ------------------------------
// Classes and structs

namespace mohair {

  struct PlanMessage {
    unique_ptr<Plan> payload;
    int              root_relndx { -1 }; // initialized to -1 as a sentinel
    PlanRel*         root_relation;

    virtual ~PlanMessage() = default;

    PlanMessage(unique_ptr<Plan>&& msg): payload(std::move(msg)) {}
    PlanMessage(unique_ptr<Plan>&& msg, int root_relndx)
      : payload(std::move(msg)), root_relndx(root_relndx) {
      this->root_relation = this->payload->mutable_relations(root_relndx);
    }

    PlanMessage(string&  msg): PlanMessage(SubstraitPlanFromString(msg)) {}
    PlanMessage(fstream& msg): PlanMessage(SubstraitPlanFromFile(&msg))  {}
  };


  // Forward declaration of PlanSplit for FromSplit prototype
  struct PlanSplit;

  // >> Adapter for substrait messages
  struct SubstraitMessage : PlanMessage {

    virtual ~SubstraitMessage() = default;

    SubstraitMessage(unique_ptr<Plan>&& msg): PlanMessage(std::move(msg)) {}
    SubstraitMessage(unique_ptr<Plan>&& msg, int root_relndx)
      : PlanMessage(std::move(msg), root_relndx) {}

    SubstraitMessage(string&  msg): PlanMessage(msg) {}
    SubstraitMessage(fstream& msg): PlanMessage(msg) {}

    virtual string Serialize();
    virtual bool   SerializeToFile(const char *out_fpath);

    // function implementation in plans.cpp
    virtual vector<unique_ptr<SubstraitMessage>> SubplansFromSplit(PlanSplit& split);
  };

} // namespace: mohair
