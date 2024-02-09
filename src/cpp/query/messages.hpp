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
// Classes and structs

namespace mohair {

  struct PlanMessage {
    virtual ~PlanMessage();

    PlanMessage(string&  msg);
    PlanMessage(fstream& msg);

    virtual string Serialize();

    // TODO: can't reference DecomposedPlan from here if this is a leaf compilation unit
    virtual unique_ptr<PlanMessage> MessageForSubPlan(DecomposedPlan* plansplit, int plan_ndx);
  };


  // >> Adapters for substrait
  /**
   * An adapter for sending plans as messages via substrait.
   */
  // TODO
  struct SubstraitPlanMessage : PlanMessage {
    unique_ptr<Plan> substrait_plan;
    PlanRel*         root_relation;
    int              root_relndx;

    unique_ptr<PlanMessage> MessageForSubPlan();
  };

} // namespace: mohair


namespace mohair {

  // >> Reader Functions
  unique_ptr<Plan> SubstraitPlanFromString(string &plan_msg);
  unique_ptr<Plan> SubstraitPlanFromFile(fstream *plan_fstream);

  void PrintSubstraitPlan(Plan *plan_msg);
  void PrintSubstraitRel(Rel   *rel_msg);

} // namespace: mohair