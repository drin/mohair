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

  // >> Specific translation functions (from Substrait to Mohair)
  template <typename UnaryRelMsg, typename MohairRel>
  unique_ptr<QueryOp> FromUnaryOpMsg(Rel& rel_msg, UnaryRelMsg& substrait_op) {
    auto op_input = MohairFrom(substrait_op.input());

    // op_inputs is structured as a tuple and we propagate table name
    auto&& op_inputs = { op_input };
    auto&& op_tname  = op_input->table_name;

    return std::make_unique<MohairRel>(substrait_op, rel_msg, op_inputs, op_tname);
  }

  template <typename BinaryRelMsg, typename MohairRel>
  unique_ptr<QueryOp> FromBinaryOpMsg(Rel& rel_msg, BinaryRelMsg& substrait_op) {
    auto left_input  = MohairFrom(substrait_op.left() );
    auto right_input = MohairFrom(substrait_op.right());

    // inputs is structured as a tuple and we propagate table name
    auto&& op_inputs = { left_input, right_input };
    auto&& op_tname  = left_input->table_name + "." + right_input->table_name;

    return std::make_unique<MohairRel>(substrait_op, rel_msg, op_inputs, op_tname);
  }


  unique_ptr<QueryOp> FromReadMsg(Rel& rel_msg, ReadRel& substrait_op) {
    string &&op_tname = std::move(SourceNameForRead(substrait_op));
    return std::make_unique<OpRead>(substrait_op, rel_msg, {}, op_tname);
  }


  // TODO
  unique_ptr<QueryOp> FromSkyMsg(Rel& rel_msg, SkyRel& substrait_op) {
    return nullptr;
  }


  /**
   * A function to convert a substrait `Rel` message to a Mohair `QueryOp` derived
   * class.
   */
  unique_ptr<QueryOp> MohairFrom(Rel& rel_msg) {
    switch(rel_msg.rel_type_case()) {
      // Translate pipeline-able operators
      case Rel::RelTypeCase::kProject: {
        return FromUnaryOpMsg<ProjectRel, OpProj>(rel_msg, rel_msg.project());
      }
      case Rel::RelTypeCase::kFilter: {
        return FromUnaryOpMsg<FilterRel, OpSel>(rel_msg, rel_msg.filter());
      }
      case Rel::RelTypeCase::kFetch: {
        return FromUnaryOpMsg<FetchRel, OpLimit>(rel_msg, rel_msg.fetch());
      }

      // Translate pipeline breaking operators
      case Rel::RelTypeCase::kSort: {
        return FromUnaryOpMsg<SortRel, OpSort>(rel_msg, substrait_op);
      }
      case Rel::RelTypeCase::kAggregate: {
        return FromUnaryOpMsg<AggregateRel, OpAggr>(rel_msg, rel_msg.aggregate());
      }
      case Rel::RelTypeCase::kCross: {
        return FromBinaryOpMsg<CrossRel, OpCrossJoin>(rel_msg, rel_msg.cross());
      }
      case Rel::RelTypeCase::kJoin: {
        return FromBinaryOpMsg<JoinRel, OpJoin>(rel_msg, rel_msg.join());
      }

      // hash and merge joins
      case Rel::RelTypeCase::kHashJoin: {
        return FromBinaryOpMsg<HashJoinRel, OpHashJoin>(rel_msg, rel_msg.hash_join());
      }
      case Rel::RelTypeCase::kMergeJoin: {
        return FromBinaryOpMsg<MergeJoinRel, OpMergeJoin>(rel_msg, rel_msg.merge_join());
      }

      // Translate leaf operators
      case Rel::RelTypeCase::kRead: {
        return FromReadMsg(rel_msg, rel_msg.read());
      }
      case Rel::RelTypeCase::kExtensionLeaf: {
        return FromSkyMsg(rel_msg, rel_msg.extension_leaf());
      }

      // note: may not be pipeline breaking, but we can improve that when we get there
      /* TODO: figure out how to do variable length tuple later
      case Rel::RelTypeCase::kSet: {
        return MohairFrom(rel_msg.set());
      }
      */

      // Catch all error
      default: {
        return std::make_unique<OpErr>(rel_msg, "ParseError: Unknown substrait operator");
      }
  }

  // >> Translation from Mohair to Substrait
  Rel& SubstraitFrom(unique_ptr<QueryOp>& mohair_op) {
    return mohair_op->op_wrap;
  }


  /* TODO: needs some non-trivial templating
  // >> Translation from Mohair to a PlanAnchor
  unique_ptr<PlanAnchor> PlanAnchorFrom(unique_ptr<QueryOp>& mohair_op) {
    // Create an ErrRel and set its error details
    auto anchor_relmsg = std::make_unique<ErrRel>();
    anchor_relmsg.set_err_msg("UnimplementedError: PlanAnchor must be a JoinRel");
    anchor_relmsg.set_err_code(ErrRel::ErrType::INVALID_MSG_TYPE);

    // Construct a PlanAnchor using the Rel
    auto anchor_msg = std::make_unique<PlanAnchor>();
    anchor_msg.set_allocated_anchor_rel(anchor_relmsg);
    return anchor_msg;
  }
  */

  unique_ptr<PlanAnchor> PlanAnchorFrom(unique_ptr<OpJoin>& mohair_op) {
    // Get a Rel and clear its children (an anchor doesn't need that info)
    auto anchor_relmsg = SubstraitFrom(mohair_op);
    anchor_relmsg.join().clear_left();
    anchor_relmsg.join().clear_right();

    // Construct a PlanAnchor using the Rel
    auto anchor_msg = std::make_unique<PlanAnchor>();
    anchor_msg.set_allocated_anchor_rel(anchor_relmsg);
    return anchor_msg;
  }


  string SourceNameForRead(ReadRel& substrait_op) {
    switch (substrait_op.read_type_case()) {
      case ReadRel::ReadTypeCase::kNamedTable: {
        std::stringstream tname_stream;
        const auto src_table = substrait_op.named_table;

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
