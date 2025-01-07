// ------------------------------
// License(s)
//
// >> For original duckdb code
// Copyright 2018-2024 Stichting DuckDB Foundation
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be included in all copies
// or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// >> For my modifications
//    (figure out what amount of modifications allows me to change the license at all)
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
#pragma once

#if SKYTETHER_USE_DUCKDB

  // >> DuckDB public API
  #include "duckdb.hpp"
  #include "duckdb/main/connection.hpp"

  // >> DuckDB common types
  #include "duckdb/common/helper.hpp"
  #include "duckdb/common/exception.hpp"
  #include "duckdb/common/string_util.hpp"
  #include "duckdb/common/types.hpp"
  #include "duckdb/common/types/hash.hpp"
  #include "duckdb/common/types/value.hpp"
  #include "duckdb/common/arrow/result_arrow_wrapper.hpp"
  #include "duckdb/common/enums/set_operation_type.hpp"

  // >> Internal types for internal APIs
  #include "duckdb/main/client_data.hpp"
  #include "duckdb/main/prepared_statement_data.hpp"
  
  // >> Internal relation API
  #include "duckdb/main/relation.hpp"
  #include "duckdb/main/relation/filter_relation.hpp"
  #include "duckdb/main/relation/projection_relation.hpp"
  #include "duckdb/main/relation/limit_relation.hpp"
  
  #include "duckdb/main/relation/join_relation.hpp"
  #include "duckdb/main/relation/cross_product_relation.hpp"
  
  #include "duckdb/main/relation/order_relation.hpp"
  #include "duckdb/main/relation/aggregate_relation.hpp"
  
  #include "duckdb/main/relation/setop_relation.hpp"
  
  // >> Internal planner API (logical plans)
  #include "duckdb/planner/planner.hpp"
  #include "duckdb/parser/parser.hpp"
  
  #include "duckdb/planner/expression.hpp"
  #include "duckdb/parser/expression/list.hpp"
  #include "duckdb/parser/expression/comparison_expression.hpp"
  
  // For operators
  #include "duckdb/planner/logical_operator.hpp"
  
  // TODO: see if these are necessary
  #include "duckdb/planner/bound_result_modifier.hpp"
  #include "duckdb/planner/joinside.hpp"
  #include "duckdb/planner/table_filter.hpp"

#endif
