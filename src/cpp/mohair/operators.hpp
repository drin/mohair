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
#include <functional>
#include <iostream>
#include <sstream>

// >> Aliases
using std::unique_ptr;

using std::string;
using std::tuple;


// ------------------------------
// Functions

namespace mohair {

  // ------------------------------
  // Base classes for operators

  // empty base class
  class QueryOp {};

  // classes for distinguishing pipeline-able operators from pipeline breakers
  class PipelineOp : QueryOp {
    const string ViewStr() { return u8"← {self}"; }
  };

  class BreakerOp : QueryOp {
    const string ViewStr() { return u8"↤ {self}"; }
  };


  // ------------------------------
  // Operator Classes

  // >> Leaf operators

  struct OpRead : PipelineOp {
    substrait::ReadRel plan_op;
    tuple<>            op_input;
    unique_ptr<string> table_name;
  };

  // TODO: figure out how this should bridge to skytether
  struct OpSkyRead : PipelineOp {
    substrait::SkyRel  plan_op;
    tuple<>            op_input;
    unique_ptr<string> table_name;
  };

  // >> Pipeline-able operators
  struct OpProj : PipelineOp {
    substrait::ProjectRel plan_op;
    tuple<QueryOp>        op_input;
    unique_ptr<string>    table_name;
  };

  struct OpSel : PipelineOp {
    substrait::FilterRel plan_op;
    tuple<QueryOp>       op_input;
    unique_ptr<string>   table_name;
  };

  struct OpLimit : PipelineOp {
    substrait::FetchRel plan_op;
    tuple<QueryOp>      op_input;
    unique_ptr<string>  table_name;
  };


  // >> Pipeline breaking operators
  struct OpSort : BreakerOp {
    substrait::SortRel plan_op;
    tuple<QueryOp>     op_input;
    unique_ptr<string> table_name;
  };


  // >> Translation functions
  QueryOp MohairFrom(substrait::Rel substrait_op_wrap) {
    // Translate pipeline-able operators
    if      (substrait_op_wrap.has_project())    { return MohairFrom(substrait_op_wrap.project());    }
    else if (substrait_op_wrap.has_filter())     { return MohairFrom(substrait_op_wrap.filter());     }
    else if (substrait_op_wrap.has_fetch())      { return MohairFrom(substrait_op_wrap.fetch());      }

    // Translate pipeline breaking operators
    else if (substrait_op_wrap.has_aggregate())  { return MohairFrom(substrait_op_wrap.aggregate());  }
    else if (substrait_op_wrap.has_sort())       { return MohairFrom(substrait_op_wrap.sort());       }
    else if (substrait_op_wrap.has_join())       { return MohairFrom(substrait_op_wrap.join());       }
    else if (substrait_op_wrap.has_cross())      { return MohairFrom(substrait_op_wrap.cross());      }

    else if (substrait_op_wrap.has_hash_join())  { return MohairFrom(substrait_op_wrap.hash_join());  }
    else if (substrait_op_wrap.has_merge_join()) { return MohairFrom(substrait_op_wrap.merge_join()); }

    // note: may not be pipeline breaking, but we can improve that when we get there
    else if (substrait_op_wrap.has_set())        { return MohairFrom(substrait_op_wrap.set());        }

    // Translate leaf operators
    else if (substrait_op_wrap.has_read())    { return MohairFrom(substrait_op_wrap.read()); }

    else if (substrait_op_wrap.has_extension_leaf()) {
      return MohairFrom(substrait_op_wrap.extension_leaf());
    }

    // Catch all: figure out how to error
    else {}
  }

  QueryOp MohairFrom(substrait::ProjectRel substrait_op) {

  }

} // namespace: mohair
