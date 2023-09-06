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

// type aliases (e.g. for substrait) included in this header
#include "operators.hpp"


// ------------------------------
// Classes and Functions

namespace mohair {

  // >> Accessors for OpVariant that use visitors
  string OpVariant::table_name() {
    // auto& rel_op should be a unique_ptr&
    auto tname_visitor = [](const auto& rel_op) { return rel_op->table_name; }

    return std::visit(tname_visitor, this->op);
  }

  const Rel* OpVariant::op_wrap() {
    // auto& rel_op should be a unique_ptr&
    auto wrap_visitor = [](const auto& rel_op) { return rel_op->op_wrap; }

    return std::visit(wrap_visitor, this->op);
  }

  unique_ptr<PlanAnchor> OpVariant::plan_anchor() {
    return std::visit(OpVariant::PlanAnchorVisitor, this->op);
  }


  // >> Specific translation functions (from Substrait to Mohair)
  template <typename UnaryRelMsg, typename MohairRel>
  OpVariant FromUnaryOpMsg(Rel *rel_msg, UnaryRelMsg *substrait_op) {
    // recurse on input relation
    auto op_input = MohairFrom(substrait_op->mutable_input());

    // rvals for constructor args
    tuple<OpVariant> &&op_inputs = { op_input };
    auto             &&op_tname  = op_input->table_name();

    // inline construction of  OpVariant (variant wrapper around a unique pointer)
    return OpVariant { 
       std::in_place_type<unique_ptr<MohairRel>>
      ,std::make_unique<MohairRel>(substrait_op, rel_msg, op_inputs, op_tname)
    };
  }

  template <typename BinaryRelMsg, typename MohairRel>
  OpVariant FromBinaryOpMsg(Rel *rel_msg, BinaryRelMsg *substrait_op) {
    // recurse on input relations
    auto left_input  = MohairFrom(substrait_op->mutable_left() );
    auto right_input = MohairFrom(substrait_op->mutable_right());

    // rvals for constructor args
    tuple<OpVariant, OpVariant> &&op_inputs = { left_input, right_input };
    auto &&op_tname = left_input->table_name() + "." + right_input->table_name();

    // inline construction of  OpVariant (variant wrapper around a unique pointer)
    return OpVariant {
       std::in_place_type<unique_ptr<MohairRel>>
      ,std::make_unique<MohairRel>(substrait_op, rel_msg, op_inputs, op_tname)
    };
  }


  OpVariant FromReadMsg(Rel *rel_msg, ReadRel *substrait_op) {
    string &&op_tname = std::move(SourceNameForRead(substrait_op));

    return OpVariant {
       std::in_place_type<unique_ptr<OpRead>>
      ,std::make_unique<OpRead>(substrait_op, rel_msg, {}, op_tname)
    };
  }


  OpVariant FromSkyMsg(Rel *rel_msg, SkyRel *substrait_op) {
    // TODO: implementation
    // TODO: validate inline construction of unique pointer
    return OpVariant {
       std::in_place_type<unique_ptr<OpSkyRead>>
      ,unique_ptr<OpSkyRead>{}
    };
  }


  /**
   * A function to convert a substrait `Rel` message to a Mohair `QueryOp` derived
   * class.
   */
  OpVariant MohairFrom(const Rel *rel_msg) {
    switch(rel_msg.rel_type_case()) {
      // Translate pipeline-able operators
      case Rel::RelTypeCase::kProject: {
        return FromUnaryOpMsg<ProjectRel, OpProj>(rel_msg, rel_msg->mutable_project());
      }
      case Rel::RelTypeCase::kFilter: {
        return FromUnaryOpMsg<FilterRel, OpSel>(rel_msg, rel_msg->mutable_filter());
      }
      case Rel::RelTypeCase::kFetch: {
        return FromUnaryOpMsg<FetchRel, OpLimit>(rel_msg, rel_msg->mutable_fetch());
      }

      // Translate pipeline breaking operators
      case Rel::RelTypeCase::kSort: {
        return FromUnaryOpMsg<SortRel, OpSort>(rel_msg, rel_msg->mutable_sort());
      }
      case Rel::RelTypeCase::kAggregate: {
        return FromUnaryOpMsg<AggregateRel, OpAggr>(rel_msg, rel_msg->mutable_aggregate());
      }
      case Rel::RelTypeCase::kCross: {
        return FromBinaryOpMsg<CrossRel, OpCrossJoin>(rel_msg, rel_msg->mutable_cross());
      }
      case Rel::RelTypeCase::kJoin: {
        return FromBinaryOpMsg<JoinRel, OpJoin>(rel_msg, rel_msg->mutable_join());
      }

      // hash and merge joins
      case Rel::RelTypeCase::kHashJoin: {
        return FromBinaryOpMsg<HashJoinRel, OpHashJoin>(
          rel_msg, rel_msg->mutable_hash_join()
        );
      }
      case Rel::RelTypeCase::kMergeJoin: {
        return FromBinaryOpMsg<MergeJoinRel, OpMergeJoin>(
          rel_msg, rel_msg->mutable_merge_join()
        );
      }

      // Translate leaf operators
      case Rel::RelTypeCase::kRead: {
        return FromReadMsg(rel_msg, rel_msg->mutable_read());
      }
      case Rel::RelTypeCase::kExtensionLeaf: {
        return FromSkyMsg(rel_msg, rel_msg->mutable_extension_leaf());
      }

      // note: may not be pipeline breaking, but we can improve that when we get there
      /* TODO: figure out how to do variable length tuple later
      case Rel::RelTypeCase::kSet: {
        return MohairFrom(rel_msg.set());
      }
      */

      // Catch all error
      default: {
        return OpVariant {
           std::in_place_type<unique_ptr<OpErr>>
          ,std::make_unique<OpErr>(rel_msg, "ParseError: Unknown substrait operator")
        };
      }
    }
  }

  // >> Translation from Mohair to Substrait
  const Rel* SubstraitFrom(OpVariant& mohair_op) {
    return mohair_op.op_wrap();
  }

  unique_ptr<PlanAnchor> PlanAnchorFrom(OpVariant& mohair_op) {
    return mohair_op.plan_anchor();
  }

  string SourceNameForRead(const ReadRel *substrait_op) {
    switch (substrait_op->read_type_case()) {
      case ReadRel::ReadTypeCase::kNamedTable: {
        std::stringstream tname_stream;
        const auto src_table = substrait_op->named_table();

        tname_stream << src_table.names(0);
        for (int tname_ndx = 1; tname_ndx < src_table.names_size(); ++tname_ndx) {
          tname_stream << "." << src_table.names(tname_ndx);
        }

        return tname_stream.str();
      }

      // all other cases can get a default name for now
      default: {
        return "ReadRel";
      }
    }
  }

} // namespace: mohair
