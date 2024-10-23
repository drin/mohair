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
// Mohair is a protocol that extends substrait in order to propagate query plans
// (represented using substrait) between query engines. Each query engine that uses the
// mohair protocol should cooperate on query processing and execution.


// ------------------------------
// Dependencies
#pragma once

// >> Common internal deps
#include "mohair.hpp"

// >> Definitions for mohair protocol
#include "skytether/mohair/algebra.pb.h"
#include "skytether/mohair/topology.pb.h"

// >> Third-party deps
#include "query/apidep_substrait.hpp"


// ------------------------------
// Type Aliases

// >> Mohair query processing types
using skytether::mohair::PlanAnchor;
using skytether::mohair::ErrRel;

// >> Mohair topology types
using skytether::mohair::ServiceConfig;
using skytether::mohair::DeviceClass;

