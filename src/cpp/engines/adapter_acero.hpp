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
#include "mohair.hpp"

//  >> Acero deps
#include <arrow/engine/api.h>

//    |> NOTE: arrow/acero/api.h is in 13.0.0 (dev) but not 12.0.1 (stable)
//    |> TODO: this should handle version differences with macros
// #include <arrow/acero/api.h>
#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>


// ------------------------------
// Type aliases

//  >> Acero-substrait types
using arrow::engine::PlanInfo;

using arrow::engine::NamedTableProvider;
using arrow::engine::ConversionOptions;
using arrow::engine::ExtensionIdRegistry;
using arrow::engine::ExtensionSet;

//  >> Acero types
using arrow::acero::Declaration;
using arrow::acero::TableSourceNodeOptions;
using arrow::acero::QueryOptions;
