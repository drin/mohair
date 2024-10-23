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

  // >> Implementations for each op type to return its string representation
  const string OpErr::ToString()       { return u8"Err()";                         }
  const string OpRead::ToString()      { return u8"Read(" + table_name + u8")";    }
  const string OpProj::ToString()      { return u8"Π("    + table_name + u8")";    }
  const string OpSel::ToString()       { return u8"σ("    + table_name + u8")";    }
  const string OpLimit::ToString()     { return u8"Lim("  + table_name + u8")";    }
  const string OpSort::ToString()      { return u8"Sort(" + table_name + u8")";    }
  const string OpAggr::ToString()      { return u8"Aggr(" + table_name + u8")";    }

  const string OpCrossJoin::ToString() { return u8"×("    + table_name + u8")";    }
  const string OpJoin::ToString()      { return u8"⋈("    + table_name + u8")";    }
  const string OpHashJoin::ToString()  { return u8"⋈→("   + table_name + u8")";    }
  const string OpMergeJoin::ToString() { return u8"⋈⊕("   + table_name + u8")";    }

  const string OpSkyRead::ToString()   { return u8"SkyRead(" + table_name + u8")"; }

  // >> Implementations for each op type to return its PlanAnchor
  unique_ptr<PlanAnchor> PlanAnchorForRel(Rel *anchor_relmsg) {
    // wrap Rel in a new PlanAnchor message
    auto anchor_msg = std::make_unique<PlanAnchor>();
    anchor_msg->set_allocated_anchor_rel(anchor_relmsg);

    // Then return the constructed message
    return anchor_msg;
  }

  unique_ptr<PlanAnchor> OpProj::ToPlanAnchor() {
    // copy the Rel message and simplify its content (we don't need its full tree)
    auto simplified_rel = std::make_unique<Rel>(*(this->op_wrap));
    simplified_rel->mutable_project()->clear_input();

    // Then return the PlanAnchor message
    return PlanAnchorForRel(simplified_rel.release());
  }

  unique_ptr<PlanAnchor> OpSel::ToPlanAnchor() {
    // copy the Rel message and simplify its content (we don't need its full tree)
    auto simplified_rel = std::make_unique<Rel>(*(this->op_wrap));
    simplified_rel->mutable_filter()->clear_input();

    // Then return the PlanAnchor message
    return PlanAnchorForRel(simplified_rel.release());
  }

  unique_ptr<PlanAnchor> OpLimit::ToPlanAnchor() {
    // copy the Rel message and simplify its content (we don't need its full tree)
    auto simplified_rel = std::make_unique<Rel>(*(this->op_wrap));
    simplified_rel->mutable_fetch()->clear_input();

    // Then return the PlanAnchor message
    return PlanAnchorForRel(simplified_rel.release());
  }

  unique_ptr<PlanAnchor> OpSort::ToPlanAnchor() {
    // copy the Rel message and simplify its content (we don't need its full tree)
    auto simplified_rel = std::make_unique<Rel>(*(this->op_wrap));
    simplified_rel->mutable_sort()->clear_input();

    // Then return the PlanAnchor message
    return PlanAnchorForRel(simplified_rel.release());
  }

  unique_ptr<PlanAnchor> OpAggr::ToPlanAnchor() {
    // copy the Rel message and simplify its content (we don't need its full tree)
    auto simplified_rel = std::make_unique<Rel>(*(this->op_wrap));
    simplified_rel->mutable_aggregate()->clear_input();

    // Then return the PlanAnchor message
    return PlanAnchorForRel(simplified_rel.release());
  }

  unique_ptr<PlanAnchor> OpCrossJoin::ToPlanAnchor() {
    // copy the Rel message and simplify its content (we don't need its full tree)
    auto simplified_rel = std::make_unique<Rel>(*(this->op_wrap));
    simplified_rel->mutable_cross()->clear_left();
    simplified_rel->mutable_cross()->clear_right();

    // Then return the PlanAnchor message
    return PlanAnchorForRel(simplified_rel.release());
  }

  unique_ptr<PlanAnchor> OpJoin::ToPlanAnchor() {
    // copy the Rel message and simplify its content (we don't need its full tree)
    auto simplified_rel = std::make_unique<Rel>(*(this->op_wrap));
    simplified_rel->mutable_join()->clear_left();
    simplified_rel->mutable_join()->clear_right();

    // Then return the PlanAnchor message
    return PlanAnchorForRel(simplified_rel.release());
  }

  unique_ptr<PlanAnchor> OpHashJoin::ToPlanAnchor() {
    // copy the Rel message and simplify its content (we don't need its full tree)
    auto simplified_rel = std::make_unique<Rel>(*(this->op_wrap));
    simplified_rel->mutable_hash_join()->clear_left();
    simplified_rel->mutable_hash_join()->clear_right();

    // Then return the PlanAnchor message
    return PlanAnchorForRel(simplified_rel.release());
  }

  unique_ptr<PlanAnchor> OpMergeJoin::ToPlanAnchor() {
    // copy the Rel message and simplify its content (we don't need its full tree)
    auto simplified_rel = std::make_unique<Rel>(*(this->op_wrap));
    simplified_rel->mutable_merge_join()->clear_left();
    simplified_rel->mutable_merge_join()->clear_right();

    // Then return the PlanAnchor message
    return PlanAnchorForRel(simplified_rel.release());
  }

  // >> End of plan_anchor() implementations


  // >> Implementations for each QueryOp to return its inputs
  using QueryOpVec = std::vector<QueryOp *>;

  // NOTE: QueryOpVec is a convenience alias in `operators.hpp`
  template <typename UnaryQueryOp>
  QueryOpVec GetInputsUnary(UnaryQueryOp *op) {
    /*
    auto &op_input_ptr = std::get<0>(op->op_inputs);
    auto op_input      = op_input_ptr.get();
    QueryOpVec in_op_list { op_input };
    return in_op_list;
    */
    return QueryOpVec { std::get<0>(op->op_inputs).get() };
  }

  QueryOpVec OpProj::GetOpInputs()  { return GetInputsUnary(this); }
  QueryOpVec OpSel::GetOpInputs()   { return GetInputsUnary(this); }
  QueryOpVec OpLimit::GetOpInputs() { return GetInputsUnary(this); }
  QueryOpVec OpSort::GetOpInputs()  { return GetInputsUnary(this); }
  QueryOpVec OpAggr::GetOpInputs()  { return GetInputsUnary(this); }


  template <typename BinaryQueryOp>
  QueryOpVec GetInputsBinary(BinaryQueryOp *op) {
    return QueryOpVec {
       std::get<0>(op->op_inputs).get()
      ,std::get<1>(op->op_inputs).get()
    };
  }

  QueryOpVec OpJoin::GetOpInputs()      { return GetInputsBinary(this); }
  QueryOpVec OpCrossJoin::GetOpInputs() { return GetInputsBinary(this); }
  QueryOpVec OpHashJoin::GetOpInputs()  { return GetInputsBinary(this); }
  QueryOpVec OpMergeJoin::GetOpInputs() { return GetInputsBinary(this); }

  // >> End of op_inputs() implementations

  // >> Specific translation functions (from Substrait to Mohair)
  template <typename UnaryRelMsg, typename MohairRel>
  unique_ptr<QueryOp> FromUnaryOpMsg(Rel *rel_msg, UnaryRelMsg *substrait_op) {
    // recurse on input relation
    unique_ptr<QueryOp> op_input = MohairFrom(substrait_op->mutable_input());

    // Initialize op and set its inputs
    auto unary_op = std::make_unique<MohairRel>(
      substrait_op, rel_msg, op_input->table_name
    );
    get<0>(unary_op->op_inputs) = std::move(op_input);

    return unary_op;
  }

  template <typename BinaryRelMsg, typename MohairRel>
  unique_ptr<QueryOp> FromBinaryOpMsg(Rel *rel_msg, BinaryRelMsg *substrait_op) {
    // recurse on input relations
    unique_ptr<QueryOp> left_input  = MohairFrom(substrait_op->mutable_left() );
    unique_ptr<QueryOp> right_input = MohairFrom(substrait_op->mutable_right());

    // prep params for the operator
    string op_tname { left_input->table_name + "." + right_input->table_name  };
    auto binary_op = std::make_unique<MohairRel>(
       substrait_op, rel_msg, op_tname
    );

    get<0>(binary_op->op_inputs) = std::move(left_input);
    get<1>(binary_op->op_inputs) = std::move(right_input);

    return binary_op;
  }


  unique_ptr<LocalFiles> LocalFileWithName(string fname) {
    // Create the `LocalFiles` we'll eventually return
    auto readfile_msg = std::make_unique<ReadRel::LocalFiles>();

    // Add a single `FileOrFiles`; producer shouldn't know physical design
    FileOrFiles* sky_partition = readfile_msg->add_items();

    // Set attributes of `FileOrFiles`:
    //  the table name is the path
    sky_partition->set_uri_path(fname);

    //  we set ArrowReadOptions to flag it as Arrow format
    auto arrow_fileopts = std::make_unique<ArrowReadOptions>();
    sky_partition->set_allocated_arrow(arrow_fileopts.release());

    // assign and return
    return readfile_msg;
  }

  unique_ptr<QueryOp> FromReadMsg(Rel *rel_msg, ReadRel *substrait_op) {
    // Initialize table name to a default value
    string op_tname { "ReadRel" };

    switch (substrait_op->read_type_case()) {
      case ReadRel::ReadTypeCase::kNamedTable: {
        auto& src_table = substrait_op->named_table();

        // Grab the parts of the table name, but ibis only encodes 1.
        std::stringstream tname_stream;
        tname_stream << src_table.names(0);
        for (int tname_ndx = 1; tname_ndx < src_table.names_size(); ++tname_ndx) {
          tname_stream << "." << src_table.names(tname_ndx);
        }

        // Set the table name
        op_tname = string { tname_stream.str() };

        // TODO: For now, convert `NamedTable` to `LocalFiles`
        std::cerr << "Converting NamedTable to LocalFiles" << std::endl;
        auto local_files_message = LocalFileWithName(op_tname);
        substrait_op->set_allocated_local_files(local_files_message.release());
        std::cerr << "Converted." << std::endl;
        break;
      }

      case ReadRel::ReadTypeCase::kLocalFiles: {
        auto& src_files = substrait_op->local_files();

        // For mohair, we only ever expect to receive single URI paths
        if (src_files.items_size() > 1) {
          std::cerr << "Error: mohair should only specify 1 File per ReadRel" << std::endl;
          return nullptr;
        }

        auto& src_file = src_files.items(0);
        if (not src_file.has_uri_path()) {
          std::cerr << "Error: mohair should only specify URI path" << std::endl;
          return nullptr;
        }

        if (not src_file.has_arrow()) {
          std::cerr << "Error: currently, mohair only supports Arrow files" << std::endl;
          return nullptr;
        }

        op_tname = string { src_file.uri_path() };
        break;
      }

      // all other cases can get a default name for now
      case ReadRel::ReadTypeCase::kVirtualTable:
          std::cerr << "Error: unexpected ReadRel type 'VirtualTable'" << std::endl;

      case ReadRel::ReadTypeCase::kExtensionTable:
          std::cerr << "Error: unexpected ReadRel type 'ExtensionTable'" << std::endl;

      default:
          return nullptr;
    }

    return std::make_unique<OpRead>(substrait_op, rel_msg, op_tname);
  }

  unique_ptr<QueryOp> FromSkyMsg(Rel* rel_msg, ExtensionLeafRel* substrait_op)  {
    auto sky_rel = std::make_unique<SkyRel>();

    // If we're translating a message with a SkyRel, unpack it
    if (substrait_op->has_detail()) { substrait_op->detail().UnpackTo(sky_rel.get()); }
    else { std::cerr << "Found ExtensionLeafRel without data" << std::endl; }

    string op_tname { sky_rel->domain() + "-" + sky_rel->partition() };
    return std::make_unique<OpSkyRead>(
      substrait_op, rel_msg, std::move(sky_rel), op_tname
    );
  }


  /**
   * A function to convert a substrait `Rel` message to a Mohair `QueryOp` derived
   * class.
   */
  unique_ptr<QueryOp> MohairFrom(Rel *rel_msg) {
    switch(rel_msg->rel_type_case()) {
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

      case Rel::RelTypeCase::kJoin: {
        return FromBinaryOpMsg<JoinRel, OpJoin>(rel_msg, rel_msg->mutable_join());
      }

      case Rel::RelTypeCase::kCross: {
        return FromBinaryOpMsg<CrossRel, OpCrossJoin>(rel_msg, rel_msg->mutable_cross());
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
        std::cout << "Found ExtensionLeaf operator" << std::endl;
        return FromSkyMsg(rel_msg, rel_msg->mutable_extension_leaf());
      }

      // Catch all error
      default: {
        return std::make_unique<OpErr>(
          rel_msg, "ParseError: conversion for operator not yet implemented"
        );
      }
    }
  }

  // >> Translation from Mohair to Substrait
  unique_ptr<PlanAnchor> PlanAnchorFrom(QueryOp* mohair_op) {
    return mohair_op->ToPlanAnchor();
  }


  // >> Convenience functions

  string SourceNameForRead(ReadRel *substrait_op) {
    switch (substrait_op->read_type_case()) {
      case ReadRel::ReadTypeCase::kNamedTable: {
        std::stringstream tname_stream;
        auto src_table = substrait_op->named_table();

        tname_stream << src_table.names(0);
        for (int tname_ndx = 1; tname_ndx < src_table.names_size(); ++tname_ndx) {
          tname_stream << "." << src_table.names(tname_ndx);
        }

        return string { tname_stream.str() };
      }

      // all other cases can get a default name for now
      default: {
        return string { "ReadRel" };
      }
    }
  }

} // namespace: mohair
