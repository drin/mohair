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

// >> Standard libs
#include <functional>
#include <sstream>
#include <variant>

// >> Internal libs
#include "../headers/mohair.hpp"

// >> Type Aliases

//  |> For standard libs
using std::tuple;
using std::get;
using std::get_if;

//  |> For substrait
using substrait::ProjectRel;
using substrait::FilterRel;
using substrait::FetchRel;

using substrait::SortRel;
using substrait::AggregateRel;

using substrait::CrossRel;
using substrait::JoinRel;

using substrait::HashJoinRel;
using substrait::MergeJoinRel;

using substrait::ReadRel;

using mohair::SkyRel;


// ------------------------------
// Functions

namespace mohair {

  // ------------------------------
  // Operator Classes

  // >> Leaf operators
  struct OpErr : QueryOp {
    const Rel *plan_op;
    string     err_msg;
  };

  struct OpRead : PipelineOp {
    const ReadRel *plan_op;
    const Rel     *op_wrap;
    tuple<>        op_inputs;
    string         table_name;
  };

  // TODO: figure out how this should bridge to skytether
  struct OpSkyRead : PipelineOp {
    const SkyRel *plan_op;
    const Rel    *op_wrap;
    tuple<>       op_inputs;
    string        table_name;
  };

  // >> Pipeline-able operators
  template <typename InputType>
  struct OpProj : PipelineOp {
    const ProjectRel *plan_op;
    const Rel        *op_wrap;
    tuple<OpVariant>  op_inputs;
    string            table_name;
  };

  template <typename InputType>
  struct OpSel : PipelineOp {
    const FilterRel  *plan_op;
    const Rel        *op_wrap;
    tuple<OpVariant>  op_inputs;
    string            table_name;
  };

  template <typename InputType>
  struct OpLimit : PipelineOp {
    const FetchRel   *plan_op;
    const Rel        *op_wrap;
    tuple<OpVariant>  op_inputs;
    string            table_name;
  };


  // >> Pipeline breaking operators
  template <typename InputType>
  struct OpSort : BreakerOp {
    const SortRel    *plan_op;
    const Rel        *op_wrap;
    tuple<OpVariant>  op_inputs;
    string            table_name;
  };

  template <typename InputType>
  struct OpAggr : BreakerOp {
    const AggregateRel *plan_op;
    const Rel          *op_wrap;
    tuple<OpVariant>    op_inputs;
    string              table_name;
  };

  template <typename InputType>
  struct OpCrossJoin : BreakerOp {
    const CrossRel              *plan_op;
    const Rel                   *op_wrap;
    tuple<OpVariant, OpVariant>  op_inputs;
    string                       table_name;
  };

  template <typename InputType>
  struct OpJoin : BreakerOp {
    const JoinRel               *plan_op;
    const Rel                   *op_wrap;
    tuple<OpVariant, OpVariant>  op_inputs;
    string                       table_name;
  };

  template <typename InputType>
  struct OpHashJoin : BreakerOp {
    const HashJoinRel           *plan_op;
    const Rel                   *op_wrap;
    tuple<OpVariant, OpVariant>  op_inputs;
    string                       table_name;
  };

  template <typename InputType>
  struct OpMergeJoin : BreakerOp {
    const MergeJoinRel          *plan_op;
    const Rel                   *op_wrap;
    tuple<OpVariant, OpVariant>  op_inputs;
    string                       table_name;
  };

  /* TODO: needs variadic op_inputs
  struct OpSet : BreakerOp {
    SetRel                *plan_op;
    Rel                   *op_wrap;
    tuple<OpVariant, ...>  op_inputs;
    unique_ptr<string>     table_name;
  };
  */


  // >> Wrapper around operator variant
  struct OpVariant {

    // Enum of operator types that may be stored
    enum VariantKind {
      ERR  ,
      READ , SKY_READ,
      PROJ , SEL     , LIMIT,
      SORT , AGGR    ,
      CROSS, JOIN    ,
      HASHJ, MERGEJ  ,
      SET  ,
    };

    // Type alias for a variant
    using VariantType = std::variant<
      unique_ptr<OpErr>      ,
      unique_ptr<OpRead>     , unique_ptr<OpSkyRead>  ,
      unique_ptr<OpProj>     , unique_ptr<OpSel>      , unique_ptr<OpLimit>,
      unique_ptr<OpSort>     , unique_ptr<OpAggr>     ,
      unique_ptr<OpCrossJoin>, unique_ptr<OpJoin>     ,
      unique_ptr<OpHashJoin> , unique_ptr<OpMergeJoin>
      // unique_ptr<OpSet>
    >;

    // Declaration of member variable of variant type
    VariantType op;

    // Convenience functions (the purpose of this wrapper)
    OpVariant::VariantKind kind() const {
      switch (this->op.index()) {
        case  1: return OpVariant::READ;
        case  2: return OpVariant::SKY_READ;
        case  3: return OpVariant::PROJ;
        case  4: return OpVariant::SEL;
        case  5: return OpVariant::LIMIT;
        case  6: return OpVariant::SORT;
        case  7: return OpVariant::AGGR;
        case  8: return OpVariant::CROSS;
        case  9: return OpVariant::JOIN;
        case 10: return OpVariant::HASHJ;
        case 11: return OpVariant::MERGEJ;
        case 12: return OpVariant::SET;
        case  0:
        default:
          return OpVariant::ERR;
      }
    }

    struct PlanAnchorVisitor {
      /**
        * Construct a PlanAnchor using the `Rel` parameter.
        */
      unique_ptr<PlanAnchor> PlanAnchorForRel(Rel *anchor_relmsg) {
        auto anchor_msg = std::make_unique<PlanAnchor>();
        anchor_msg.set_allocated_anchor_rel(anchor_relmsg);

        return anchor_msg;
      }

      // >> Visitor functions for each type of mohair operator
      unique_ptr<PlanAnchor> operator()(unique_ptr<OpRead>& op) {
        return PlanAnchorForRel(SubstraitFrom(*op));
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpSkyRead>& op) {
        return PlanAnchorForRel(SubstraitFrom(*op));
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpProj>& op) {
        auto anchor_relmsg = SubstraitFrom(*op);
        anchor_relmsg->project().clear_input();

        return PlanAnchorForRel(anchor_relmsg);
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpSel>& op) {
        auto anchor_relmsg = SubstraitFrom(*op);
        anchor_relmsg->filter().clear_input();

        return PlanAnchorForRel(anchor_relmsg);
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpLimit>& op) {
        auto anchor_relmsg = SubstraitFrom(*op);
        anchor_relmsg->fetch().clear_input();

        return PlanAnchorForRel(anchor_relmsg);
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpSort>& op) {
        auto anchor_relmsg = SubstraitFrom(*op);
        anchor_relmsg->sort().clear_input();

        return PlanAnchorForRel(anchor_relmsg);
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpAggr>& op) {
        auto anchor_relmsg = SubstraitFrom(*op);
        anchor_relmsg->aggregate().clear_input();

        return PlanAnchorForRel(anchor_relmsg);
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpCrossJoin>& op) {
        auto anchor_relmsg = SubstraitFrom(*op);
        anchor_relmsg->cross().clear_left();
        anchor_relmsg->cross().clear_right();

        return PlanAnchorForRel(anchor_relmsg);
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpJoin>& op) {
        auto anchor_relmsg = SubstraitFrom(*op);
        anchor_relmsg->join().clear_left();
        anchor_relmsg->join().clear_right();

        return PlanAnchorForRel(anchor_relmsg);
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpHashJoin>& op) {
        auto anchor_relmsg = SubstraitFrom(*op);
        anchor_relmsg->hash_join().clear_left();
        anchor_relmsg->hash_join().clear_right();

        return PlanAnchorForRel(anchor_relmsg);
      }

      unique_ptr<PlanAnchor> operator()(unique_ptr<OpMergeJoin>& op) {
        auto anchor_relmsg = SubstraitFrom(*op);
        anchor_relmsg->merge_join().clear_left();
        anchor_relmsg->merge_join().clear_right();

        return PlanAnchorForRel(anchor_relmsg);
      }

    }; // struct PlanAnchorVisitor

    // >> Accessors for OpVariant that use visitors
    string table_name();
    const Rel* op_wrap();
    unique_ptr<PlanAnchor> plan_anchor();

  }; // struct OpVariant

  // >> Convenience functions
  string SourceNameForRead(const ReadRel *substrait_op);

} // namespace: mohair
