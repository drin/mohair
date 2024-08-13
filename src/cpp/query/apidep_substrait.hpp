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
// Overview
//
// Substrait is a specification of how to represent a query plan. This implementation uses
// protobuf definitions provided by substrait.


// ------------------------------
// Dependencies
#pragma once

// >> Protobuf deps for interacting with framework
#include <google/protobuf/text_format.h>

// >> Generated protobuf deps for substrait
#include "substrait/plan.pb.h"
#include "substrait/algebra.pb.h"
#include "substrait/extensions/extensions.pb.h"


// ------------------------------
// Type Aliases

//  >> Protobuf framework types
using google::protobuf::TextFormat;

//  >> Substrait types
using substrait::Plan;
using substrait::PlanRel;
using substrait::Rel;

