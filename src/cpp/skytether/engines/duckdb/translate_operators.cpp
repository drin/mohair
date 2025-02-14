// ------------------------------
// License(s)
//
// >> DuckDB license
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
// >> Internal license
// Copyright 2024-2025 Aldrin Montana
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

#include "skytether/engines/duckdb/adapter_duckdb.hpp"

// TODO: clean up this relationship
#include "services/types.hpp"
#include "services/service_skytether.hpp"

#include <arrow/io/memory.h>


// ------------------------------
// Macros and Type Aliases

namespace skytether::engines {

  // >> Templated types

  // >> Protobuf types
  using RepeatedString = mohair::RepeatedPtrField<string>;

  // >> Substrait types
  using NamedTable = mohair::ReadRel::NamedTable;
  using LocalFiles = mohair::ReadRel::LocalFiles;

  // >> Flight client types
  using  services::SkytetherClient;
  using  services::Location;

} // namespace: skytether::engines


// ------------------------------
// Functions

namespace skytether::engines {

  //! Normalization of an Arrow URI path to a table name
  string TableAliasForArrowFile(const std::string& uri_path) {
    string table_alias { uri_path };

    // NOTE: replace system characters with ones that play nice at a higher level
    for (uint32_t ndx = 0; ndx < table_alias.size(); ++ndx) {
      // TODO: can't do string literal comparisons
      if      (table_alias[ndx] == '/') { table_alias[ndx] = '.'; }
      else if (table_alias[ndx] == ';') { table_alias[ndx] = '-'; }
    }

    return table_alias;
  }

  duckdb::SetOperationType
  TranslateSetOperationType(mohair::SetRel::SetOp setop) {
    switch (setop) {
      case mohair::SetRel::SET_OP_UNION_ALL: {
        return duckdb::SetOperationType::UNION;
      }

      case mohair::SetRel::SET_OP_MINUS_PRIMARY: {
        return duckdb::SetOperationType::EXCEPT;
      }

      case mohair::SetRel::SET_OP_INTERSECTION_PRIMARY: {
        return duckdb::SetOperationType::INTERSECT;
      }

      default: {
        throw duckdb::NotImplementedException(
           "SetOperationType transform not implemented for SetRel type %d"
          ,setop
        );
      }
    }
  }


  duckdb::JoinType
  TranslateJoinType(const mohair::JoinRel& sjoin) {
    switch (sjoin.type()) {
      case mohair::JoinRel::JOIN_TYPE_INNER:       return duckdb::JoinType::INNER;
      case mohair::JoinRel::JOIN_TYPE_LEFT:        return duckdb::JoinType::LEFT;
      case mohair::JoinRel::JOIN_TYPE_RIGHT:       return duckdb::JoinType::RIGHT;
      case mohair::JoinRel::JOIN_TYPE_LEFT_SINGLE: return duckdb::JoinType::SINGLE;
      case mohair::JoinRel::JOIN_TYPE_LEFT_SEMI:   return duckdb::JoinType::SEMI;
      case mohair::JoinRel::JOIN_TYPE_OUTER:       return duckdb::JoinType::OUTER;

      default:
        throw duckdb::InternalException("Unsupported join type");
    }
  }

  void
  PrintJoinType(const duckdb::JoinType jtype) {
    switch (jtype) {
      case duckdb::JoinType::INNER:
        SkytetherDebugMsg("Inner");
        break;

      case duckdb::JoinType::LEFT:
        SkytetherDebugMsg("Left");
        break;

      case duckdb::JoinType::RIGHT:
        SkytetherDebugMsg("Right");
        break;

      case duckdb::JoinType::SINGLE:
        SkytetherDebugMsg("Left single");
        break;

      case duckdb::JoinType::SEMI:
        SkytetherDebugMsg("Left semi");
        break;

      case duckdb::JoinType::OUTER:
        SkytetherDebugMsg("Outer");
        break;


      default:
        SkytetherDebugMsg("Unknown");
        break;
    }
  }


  duckdb::OrderByNode
  TranslateOrder(TranslatorState& tl_state, const SubstraitSortField& sordf) {
    duckdb::OrderType       dordertype;
    duckdb::OrderByNullType dnullorder;

    switch (sordf.direction()) {
      case SubstraitSortField::SORT_DIRECTION_ASC_NULLS_FIRST:
        dordertype = duckdb::OrderType::ASCENDING;
        dnullorder = duckdb::OrderByNullType::NULLS_FIRST;
        break;

      case SubstraitSortField::SORT_DIRECTION_ASC_NULLS_LAST:
        dordertype = duckdb::OrderType::ASCENDING;
        dnullorder = duckdb::OrderByNullType::NULLS_LAST;
        break;

      case SubstraitSortField::SORT_DIRECTION_DESC_NULLS_FIRST:
        dordertype = duckdb::OrderType::DESCENDING;
        dnullorder = duckdb::OrderByNullType::NULLS_FIRST;
        break;

      case SubstraitSortField::SORT_DIRECTION_DESC_NULLS_LAST:
        dordertype = duckdb::OrderType::DESCENDING;
        dnullorder = duckdb::OrderByNullType::NULLS_LAST;
        break;

      default:
        throw duckdb::InternalException(
          "Unsupported ordering " + duckdb::to_string(sordf.direction())
        );
    }

    return { dordertype, dnullorder, TranslateExpr(tl_state, sordf.expr()) };
  }

  // Function prototype for top-level translation function
  duck_sptr<DuckRel>
  TranslateOp(TranslatorState& tl_state, const mohair::Rel& sop);


  duck_sptr<DuckRel>
  TranslateJoinOp(TranslatorState& tl_state, const mohair::JoinRel& sjoin) {
    duckdb::JoinType            djointype      = TranslateJoinType(sjoin);
    duck_uptr<ParsedExpression> join_condition = TranslateExpr(
      tl_state, sjoin.expression()
    );

    return duckdb::make_shared_ptr<JoinRelation>(
       TranslateOp(tl_state, sjoin.left() )->Alias("left")
      ,TranslateOp(tl_state, sjoin.right())->Alias("right")
      ,std::move(join_condition)
      ,djointype
    );
  }

  duck_sptr<DuckRel>
  TranslateCrossProductOp(TranslatorState& tl_state, const mohair::CrossRel& scross) {
    return duckdb::make_shared_ptr<CrossProductRelation>(
       TranslateOp(tl_state, scross.left() )->Alias("left")
      ,TranslateOp(tl_state, scross.right())->Alias("right")
    );
  }

  // TODO: refactor count and offset to use count_expr and offset_expr
  duck_sptr<DuckRel>
  TranslateFetchOp(TranslatorState& tl_state, const mohair::FetchRel& slimit) {
    return duckdb::make_shared_ptr<LimitRelation>(
       TranslateOp(tl_state, slimit.input())
      ,slimit.count()
      ,slimit.offset()
    );
  }

  duck_sptr<DuckRel>
  TranslateFilterOp(TranslatorState& tl_state, const mohair::FilterRel& sfilter) {
    return duckdb::make_shared_ptr<FilterRelation>(
       TranslateOp(tl_state, sfilter.input())
      ,TranslateExpr(tl_state, sfilter.condition())
    );
  }

  duck_sptr<DuckRel>
  TranslateProjectOp(TranslatorState& tl_state, const mohair::ProjectRel& sproj) {
    duckdb::vector<duck_uptr<ParsedExpression>> expressions;
    for (auto &sexpr : sproj.expressions()) {
      expressions.push_back(TranslateExpr(tl_state, sexpr));
    }

    duckdb::vector<string> mock_aliases;
    for (size_t i = 0; i < expressions.size(); i++) {
      mock_aliases.push_back("expr_" + duckdb::to_string(i));
    }

    return duckdb::make_shared_ptr<ProjectionRelation>(
       TranslateOp(tl_state, sproj.input())
      ,std::move(expressions)
      ,std::move(mock_aliases)
    );
  }


  duck_sptr<DuckRel>
  TranslateAggregateOp(TranslatorState& tl_state, const mohair::AggregateRel& saggr) {
    duckdb::vector<duck_uptr<ParsedExpression>> groups, expressions;

    if (saggr.groupings_size() > 0) {
      for (auto &sgrp : saggr.groupings()) {
        for (auto &sgrpexpr : sgrp.grouping_expressions()) {
          groups.push_back(TranslateExpr(tl_state, sgrpexpr));
          expressions.push_back(TranslateExpr(tl_state, sgrpexpr));
        }
      }
    }

    for (auto &smeas : saggr.measures()) {
      duckdb::vector<duck_uptr<ParsedExpression>> children;
      for (auto &sarg : smeas.measure().arguments()) {
        children.push_back(TranslateExpr(tl_state, sarg.value()));
      }

      auto        function_id = smeas.measure().function_reference();
      const auto& fn_entry    = tl_state.fn_anchors->find(function_id);
      string fn_name { fn_entry->second };
      if (fn_name == "count" && children.empty()) {
        fn_name = string { "count_star" };
      }

      expressions.push_back(
        duckdb::make_uniq<FunctionExpression>(
           RemapFunctionName(tl_state.fn_remaps, fn_name), std::move(children)
        )
      );
    }

    return duckdb::make_shared_ptr<AggregateRelation>(
       TranslateOp(tl_state, saggr.input())
      ,std::move(expressions)
      ,std::move(groups)
    );
  }


  duck_sptr<DuckRel>
  ScanNamedTable(TranslatorState& tl_state, const NamedTable& named_table) {
      try         { return tl_state.conn->Table(named_table.names(0)); }
      catch (...) { return tl_state.conn->View (named_table.names(0)); }
  }


  duck_sptr<DuckRel>
  ScanFileListParquet(TranslatorState& tl_state, const LocalFiles& local_files) {
    duckdb::vector<Value> parquet_files;

    auto& local_file_items = local_files.items();
    for (auto &current_file : local_file_items) {
      if (current_file.has_uri_file()) {
        parquet_files.emplace_back(current_file.uri_file());
      }

      else if (current_file.has_uri_path()) {
        parquet_files.emplace_back(current_file.uri_path());
      }

      else {
        throw duckdb::NotImplementedException(
          "Unsupported type for file path, Only uri_file and uri_path are "
          "currently supported"
        );
      }
    }

    string scan_alias {
      "parquet_" + duckdb::StringUtil::GenerateRandomName()
    };

    named_parameter_map_t named_parameters({
      { "binary_as_string", Value::BOOLEAN(false) }
    });

    return (
      tl_state.conn->TableFunction(
                        "parquet_scan"
                       ,{ Value::LIST(parquet_files) }
                       ,named_parameters
                     )
                   ->Alias(scan_alias)
    );
  }


  duck_sptr<DuckRel>
  ScanFileListArrow(TranslatorState& tl_state, const LocalFiles& local_files) {
    duckdb::vector<Value> arrow_files;
    string                table_name;

    auto& local_file_items = local_files.items();
    for (auto &current_file : local_file_items) {
      if (current_file.has_uri_path()) {
        arrow_files.emplace_back(current_file.uri_path());
        table_name = TableAliasForArrowFile(current_file.uri_path());
      }

      else {
        throw duckdb::NotImplementedException(
          "Only uri_path is supported for arrow file paths"
        );
      }
    }

    // We expect the arrow files to contain arrow stream formatted data
    return (
      tl_state.conn->TableFunction("scan_arrows_file", { Value::LIST(arrow_files) })
                   ->Alias(table_name)
    );
  }


  using SFileFormatType = LocalFiles::FileOrFiles::FileFormatCase;
  duck_sptr<DuckRel>
  ScanFileList(TranslatorState& tl_state, const LocalFiles& local_files) {
    // TODO: currently, all files must be the same format
    switch (local_files.items(0).file_format_case()) {
      case SFileFormatType::kParquet:
        return ScanFileListParquet(tl_state, local_files);
        break;

      case SFileFormatType::kArrow:
        return ScanFileListArrow(tl_state, local_files);
        break;

      case SFileFormatType::kExtension:
      default:
        throw duckdb::NotImplementedException(
          "[test] Unsupported type of local file for read operator on substrait"
        );
    }
  }


  duck_sptr<DuckRel>
  TranslateReadOp(TranslatorState& tl_state, const mohair::ReadRel& sget) {

    // Construct a scan relation based on the ReadRel's source type
    duck_sptr<DuckRel> scan;
    if      (sget.has_named_table()) { scan = ScanNamedTable(tl_state, sget.named_table()); }
    else if (sget.has_local_files()) { scan =   ScanFileList(tl_state, sget.local_files()); }
    else {
      throw duckdb::NotImplementedException(
        "Unsupported type of read operator for substrait"
      );
    }

    // Filter predicate for scan operation
    if (sget.has_filter()) {
      scan = duckdb::make_shared_ptr<FilterRelation>(
        std::move(scan), TranslateExpr(tl_state, sget.filter())
      );
    }

    // Projection predicate for scan operation
    if (sget.has_projection()) {
      duckdb::vector<string>                      aliases;
      duckdb::vector<duck_uptr<ParsedExpression>> expressions;

      idx_t expr_idx = 0;
      for (auto &sproj : sget.projection().select().struct_items()) {
        // FIXME how to get actually alias?
        aliases.push_back("expr_" + duckdb::to_string(expr_idx++));

        // TODO make sure nothing else is in there
        expressions.push_back(
          duckdb::make_uniq<PositionalReferenceExpression>(sproj.field() + 1)
        );
      }

      scan = duckdb::make_shared_ptr<ProjectionRelation>(
        std::move(scan), std::move(expressions), std::move(aliases)
      );
    }

    return scan;
  }


  duck_sptr<DuckRel>
  TranslateSortOp(TranslatorState& tl_state, const mohair::SortRel &ssort) {
    duckdb::vector<duckdb::OrderByNode> order_nodes;
    for (auto &sordf : ssort.sorts()) {
      order_nodes.push_back(TranslateOrder(tl_state, sordf));
    }

    return duckdb::make_shared_ptr<OrderRelation>(
      TranslateOp(tl_state, ssort.input()), std::move(order_nodes)
    );
  }


  duck_sptr<DuckRel>
  TranslateSetOp(TranslatorState& tl_state, const mohair::SetRel &sset) {
    // TODO: see if this is necessary for some cases
    // D_ASSERT(sop.has_set());

    auto  type   = TranslateSetOperationType(sset.op());
    auto& inputs = sset.inputs();
    if (sset.inputs_size() > 2) {
      throw duckdb::NotImplementedException(
        "Too many inputs (%d) for this set operation", sset.inputs_size()
      );
    }

    auto lhs = TranslateOp(tl_state, inputs[0]);
    auto rhs = TranslateOp(tl_state, inputs[1]);
    return duckdb::make_shared_ptr<SetOpRelation>(std::move(lhs), std::move(rhs), type);
  }


  //! A SkyRel is assumed to be a lookup against a skytether catalog
  duck_sptr<DuckRel>
  TranslateSkyRel(TranslatorState& tl_state, const mohair::SkyRel& sky_rel) {
    string sky_relname { sky_rel.domain() + "/" + sky_rel.partition() };

    try         { return tl_state.conn->Table(sky_relname); }
    catch (...) { return tl_state.conn->View (sky_relname); }
  }

  //! A SkyPartitionRel is assumed to be a filesystem lookup against a partition (need to
  //  find names of each slice contained in the partition).
  duck_sptr<DuckRel>
  TranslateSkyPartitionRel( TranslatorState&               tl_state
                           ,const mohair::SkyPartitionRel& sky_rel) {
    duckdb::vector<Value> slice_names;
    string table_name { sky_rel.domain() + "/" + sky_rel.partition() };

    // In this case, we iterate over slices() to get slice indices
    // TODO: this now needs to use the `slice_selection` field
    /*
    if (not sky_rel.slices().empty()) {
      for (int slice_id = 0; slice_id < sky_rel.slices_size(); ++slice_id) {
        slice_names.emplace_back(
          table_name + "-" + duckdb::to_string(sky_rel.slices(slice_id))
        );
      }
    }
    */

    // In this case, we generate slice indices up to the last index
    // NOTE: hardcoded for now due to lazyness
    for (uint32_t slice_ndx = 0; slice_ndx < 10; ++slice_ndx) {
      slice_names.emplace_back(table_name + "-" + duckdb::to_string(slice_ndx));
    }

    // We expect the arrow files to contain arrow stream formatted data
    return (
      tl_state.conn->TableFunction("scan_arrows_file", {Value::LIST(slice_names)})
                   ->Alias(table_name)
    );
  }

  //! A SkySliceRel is assumed to be a filesystem lookup against a single slice ("as-is").
  duck_sptr<DuckRel>
  TranslateSkySliceRel(TranslatorState& tl_state, const mohair::SkySliceRel& sky_rel) {
    duckdb::vector<Value> slice_names;
    slice_names.emplace_back(sky_rel.slice_key());

    // We expect the arrow files to contain arrow stream formatted data
    string table_name { sky_rel.domain() + "/" + sky_rel.partition() };
    return (
      tl_state.conn->TableFunction("scan_arrows_file", { Value::LIST(slice_names) })
                   ->Alias(table_name)
    );
  }

  //! A SkySliceRel is assumed to be a filesystem lookup against a single slice ("as-is").
  duck_sptr<DuckRel>
  TranslateSkyResultRel(TranslatorState& tl_state, const mohair::SkyResultRel& sky_rel) {
    duckdb::vector<string>                      proj_aliases;
    duckdb::vector<duck_uptr<ParsedExpression>> proj_exprs;
    proj_exprs.emplace_back(duckdb::make_uniq<StarExpression>());

    return (
      tl_state.conn->View(sky_rel.result_name())
                   ->Project(std::move(proj_exprs), std::move(proj_aliases))
    );
  }

  duck_sptr<DuckRel>
  TranslateExtensionLeafOp(TranslatorState& tl_state, const ExtensionLeafRel& leaf_rel) {
    duck_sptr<DuckRel> translated_rel { nullptr };

    if (not leaf_rel.has_detail()) {
      throw duckdb::InternalException("ExtensionLeaf op is missing extension details");
    }

    // Figure out which type of extension operator it is
    auto& extension_msg = leaf_rel.detail();
    if (extension_msg.Is<mohair::SkyRel>()) { 
      mohair::SkyRel sky_rel;
      extension_msg.UnpackTo(&sky_rel);

      translated_rel = TranslateSkyRel(tl_state, sky_rel);
    }

    else if (extension_msg.Is<mohair::SkyPartitionRel>()) { 
      mohair::SkyPartitionRel sky_rel;
      extension_msg.UnpackTo(&sky_rel);

      translated_rel = TranslateSkyPartitionRel(tl_state, sky_rel);
    }

    else if (extension_msg.Is<mohair::SkySliceRel>()) { 
      mohair::SkySliceRel sky_rel;
      extension_msg.UnpackTo(&sky_rel);

      translated_rel = TranslateSkySliceRel(tl_state, sky_rel);
    }

    else if (extension_msg.Is<mohair::SkyResultRel>()) { 
      mohair::SkyResultRel sky_rel;
      extension_msg.UnpackTo(&sky_rel);

      translated_rel = TranslateSkyResultRel(tl_state, sky_rel);
    }

    // If a translator was matched and it succeeded, return the translated sub-plan
    if (translated_rel == nullptr) {
      SkytetherDebugMsg(
        "Unsupported extension type: " << extension_msg.descriptor()->name()
      );
      throw duckdb::InternalException("Unsupported extension type");
    }

    return translated_rel;
  }

  //! Translate Substrait Operations to DuckDB Relations
  using SRelType = mohair::Rel::RelTypeCase;
  duck_sptr<DuckRel> TranslateOp(TranslatorState& tl_state, const mohair::Rel& sop) {
    switch (sop.rel_type_case()) {
      case SRelType::kJoin:          return TranslateJoinOp         (tl_state, sop.join());
      case SRelType::kCross:         return TranslateCrossProductOp (tl_state, sop.cross());
      case SRelType::kFetch:         return TranslateFetchOp        (tl_state, sop.fetch());
      case SRelType::kFilter:        return TranslateFilterOp       (tl_state, sop.filter());
      case SRelType::kProject:       return TranslateProjectOp      (tl_state, sop.project());
      case SRelType::kAggregate:     return TranslateAggregateOp    (tl_state, sop.aggregate());
      case SRelType::kRead:          return TranslateReadOp         (tl_state, sop.read());
      case SRelType::kSort:          return TranslateSortOp         (tl_state, sop.sort());
      case SRelType::kSet:           return TranslateSetOp          (tl_state, sop.set());
      case SRelType::kExtensionLeaf: return TranslateExtensionLeafOp(tl_state, sop.extension_leaf());
      case SRelType::kReference: {
        std::cerr << "Unexpectedly found ReferenceRel." << std::endl;
        // break;
      }

      default:
        throw duckdb::InternalException(
          "Unsupported relation type " + duckdb::to_string(sop.rel_type_case())
        );
    }
  }

  //! Translates substrait `RelRoot` operator (root operator of a plan) to DuckDB
  duck_uptr<DuckRel> EngineDuckDB::TranslatePlan(SystemPlan& sys_plan) {
    SkytetherDebugMsg("Preparing for plan translation");
    mohair::PrintSubstraitPlan(sys_plan.plan_msg->payload.get());
    const RelRoot&   root_rel   { sys_plan.RootRelation()                };

    // TODO: cleanup the root projection that duckdb needs
    const RelCommon& rel_common { mohair::GetRelCommon(root_rel.input()) };
    const RepeatedString* result_names = &(rel_common.hint().output_schema().names());

    SkytetherDebugMsg(
         "[" << std::to_string(result_names->size())
      << "] attributes in final projection"
    );

    duckdb::vector<string> aliases;
    duckdb::vector<duck_uptr<ParsedExpression>> expressions;

    aliases.reserve(result_names->size());
    expressions.reserve(result_names->size());

    for (int col_ndx = 0; col_ndx < result_names->size(); ++col_ndx) {
      aliases.push_back(result_names->Get(col_ndx));
      expressions.push_back(
        duckdb::make_uniq<PositionalReferenceExpression>(1 + col_ndx)
      );
    }

    // Initialize a simple object to pass shared state
    TranslatorState tl_state { &engine_conn, &(sys_plan.fn_anchors) };

    SkytetherDebugMsg("Translating plan");
    return duckdb::make_uniq<ProjectionRelation>(
       TranslateOp(tl_state, root_rel.input())
      ,std::move(expressions)
      ,aliases
    );
  }

  unique_ptr<ExtensionLeafRel>
  EngineDuckDB::TranslateResultProjection( ProjectionRelation& result_proj
                                          ,size_t              ctx_id
                                          ,const string&       srv_loc
                                          ,const string&       result_name) {
    SkyResultRel result_rel;
    result_rel.set_context_id(ctx_id);
    result_rel.set_result_name(result_name);
    result_rel.set_service_location(srv_loc);

    // Populate `SkyResultRel`
    SkytetherDebugMsg("Creating SkyResultRel");
    SubstraitSchema*       result_schema   = result_rel.mutable_schema();
    SubstraitType::Struct* result_coltypes = result_schema->mutable_struct_();

    // TODO: figure out how to pass nullability correctly
    for (auto& duck_col : result_proj.Columns()) {
      *(result_schema->add_names())   = duck_col.Name();
      *(result_coltypes->add_types()) = *(FromDuckType(duck_col.Type(), true));
    }

    // Populate `ExtensionLeafRel`
    SkytetherDebugMsg("Creating Extension operator for SkyResultRel");
    auto leaf_rel = std::make_unique<ExtensionLeafRel>();
    leaf_rel->mutable_detail()->PackFrom(result_rel);

    RelCommon::Hint* leafrel_hint = leaf_rel->mutable_common()->mutable_hint();
    for (const auto& proj_expr : result_proj.expressions) {
      string* alias = leafrel_hint->add_output_names();
      *alias        = proj_expr->alias;
    }

    return leaf_rel;
  }

  /*
  // >> Translation back to substrait
  RelRoot* TranslateDuckRoot(LogicalOperator* duck_op) {
    unique_ptr<RelRoot> root_rel { std::make_unique<RelRoot>() };

    // This is a weird scenario where a projection is put on top of a top-k but
    // the actual aliases are on the projection below the top-k still.
    bool weird_scenario = (
         duck_op->type              == LogicalOperatorType::LOGICAL_PROJECTION
      && duck_op->children[0]->type == LogicalOperatorType::LOGICAL_TOP_N
    );

    LogicalOperator* tmp_op = duck_op;
    if (weird_scenario) { tmp_op = tmp_op->children[0].get(); }

    // Recurse through ops until the first projection; then grab output aliases
    while (tmp_op->type != LogicalOperatorType::LOGICAL_PROJECTION) {
      if (IsSetOperation(*tmp_op)) {
        // Take the projection from the first child of the set operation
        D_ASSERT(tmp_op->children.size() == 2);
        tmp_op = tmp_op->children[1].get();
        continue;
      }

      if (tmp_op->children.size() != 1) {
        throw InternalException(
          "Expected only unary operators before reaching a projection."
          "Found [%d] with [%d] children.",
          tmp_op->type, tmp_op->children.size()
        );
      }

      tmp_op = tmp_op->children[0].get();
    }

    root_rel->set_allocated_input(TranslateDuckOp(duck_op));
    auto& dproj = tmp_op->Cast<LogicalProjection>();
    if (!weird_scenario) {
      for (auto &expression : dproj.expressions) {
        root_rel->add_names(expression->GetName());

        auto depth_names = DepthFirstNames(expression->return_type);
        for (auto &name : depth_names) { root_rel->add_names(name); }
      }
    }

    else {
      for (auto &expression : duck_op->expressions) {
        auto &b_expr = expression->Cast<BoundReferenceExpression>();
        root_rel->add_names(dproj.expressions[b_expr.index]->GetName());

        auto depth_names = DepthFirstNames(expression->return_type);
        for (auto &name : depth_names) { root_rel->add_names(name); }
      }
    }

    return root_rel;
  }

  void TranslatePushback(LogicalOperator* duck_op) {
    plan.add_relations()->set_allocated_root(TranslateDuckRoot(duck_op));

    if (strict && !errors.empty()) {
      throw InvalidInputException(
          "Strict Mode is set to true, and the following warnings/errors happened. \n"
        + errors
      );
    }

    auto version = plan.mutable_version();
    version->set_major_number(0);
    version->set_minor_number(53);
    version->set_patch_number(0);


    auto* producer_name = new string();
    *producer_name = "DuckDB";
    version->set_allocated_producer(producer_name);
  }
  */

} // namespace: skytether::engines
