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
// Dependencies

#include <iterator>
#include <algorithm>

// >> Configuration-based macros
#include "skytether-config.hpp"

#include "service_duckdb.hpp"


// ------------------------------
// Classes and Methods

// >> DuckDBService implementations
namespace skytether::services {

  // Support function
  string EngineIDForLocation(const string& engine_loc) {
    string result;
    result.reserve(engine_loc.size());

    std::transform(
       engine_loc.cbegin(), engine_loc.cend(), std::back_inserter(result)
      ,[](unsigned char id_char) { return isalnum(id_char) ? id_char : '_'; }
    );

    return result;
  }

  unique_ptr<Rel>
  ResultRelWithContext(Plan* view_plan, size_t ctx_id, uint64_t view_id, const string& loc) {
    // Get the common portion of the root operator
    const PlanRel& view_planrel = view_plan->relations(0);
    if (not view_planrel.has_root()) {
      std::cerr << "Expected first relation of view plan to be a root subtree"
                << std::endl
      ;
      return nullptr;
    }

    const RelCommon& view_common = mohair::GetRelCommon(view_planrel.root().input());

    // Initialize SkyResultRel
    unique_ptr<SkyResultRel> skyrel = mohair::CreateResultRelForPlan(view_plan);
    skyrel->set_context_id(ctx_id);
    skyrel->set_result_name(std::to_string(view_id));
    skyrel->set_location(loc);

    // Initialize ExtensionLeafRel inside a Rel
    auto result_rel = std::make_unique<Rel>();
    result_rel->set_allocated_extension_leaf(new ExtensionLeafRel);

    // Then, initialize `ExtensionLeafRel`
    ExtensionLeafRel* leaf_rel  = result_rel->mutable_extension_leaf();
    RelCommon::Hint*  leaf_hint = leaf_rel->mutable_common()->mutable_hint();
    leaf_hint->mutable_output_schema()->CopyFrom(skyrel->schema());
    leaf_hint->set_alias_hash(view_id);
    leaf_rel->mutable_detail()->PackFrom(*skyrel);
    leaf_rel->mutable_common()->set_operator_id(view_common.operator_id());

    return result_rel;
  }

  unique_ptr<Plan>
  PushbackFromExecutionPlan(const SystemPlan& exec_plan, unique_ptr<Rel> result_rel) {
    // Initialize pushback from execution plan
    auto pushback = std::make_unique<Plan>();
    pushback->CopyFrom(*(exec_plan.plan_msg->payload));

    // Replace the root rel with the result rel
    for (int rel_ndx = 0; rel_ndx < pushback->relations_size(); ++rel_ndx) {
      PlanRel* subtree = pushback->mutable_relations(rel_ndx);
      if (subtree->has_root()) {
        subtree->mutable_root()->set_allocated_input(result_rel.release());
      }
    }

    // Set substrait version (TODO: double check this)
    pushback->mutable_version()->set_major_number(0);
    pushback->mutable_version()->set_major_number(53);
    pushback->mutable_version()->set_major_number(0);
    pushback->mutable_version()->set_allocated_producer(new string { "Skytether" });

    return pushback;
  }

  Result<bool> ShouldDecomposeEagerly(const DecomposeAlg& split_strat) {
    switch (split_strat) {
      case DecomposeAlg::Eager:
      case DecomposeAlg::WideJoinHead:
      case DecomposeAlg::LongPipelineHead:
      case DecomposeAlg::TallJoinLeaf:
      case DecomposeAlg::LongPipelineLeaf: return true;
      case DecomposeAlg::None:             return false;

      default:
        return Status::Invalid(
            "Unknown decomposition algorithm: "
          + skyproto::mohair::DecomposeAlg_Name(split_strat)
        );
    }
  }

  // Constructors
  DuckDBService::DuckDBService( unique_ptr<ServiceConfig>&& cfg
                               ,ShutdownCallback*           cb_custom
                               ,fs::path                    db_fpath)
    : EngineService(std::move(cfg), cb_custom) {
    string service_id = this->service_cfg->label();
    engine = skytether::engines::DuckDBForFile(service_id, db_fpath);
  }

  DuckDBService::DuckDBService( unique_ptr<ServiceConfig>&& cfg
                               ,ShutdownCallback*           cb_custom)
    : EngineService(std::move(cfg), cb_custom) {
    string service_id = this->service_cfg->label();
    engine = skytether::engines::DuckDBForMem(service_id);
  }

  DuckDBService::DuckDBService(unique_ptr<ServiceConfig>&& cfg, fs::path db_fpath)
    : DuckDBService::DuckDBService(std::move(cfg), nullptr, db_fpath) {}

  DuckDBService::DuckDBService(unique_ptr<ServiceConfig>&& cfg)
    : DuckDBService::DuckDBService(std::move(cfg), nullptr) {}


  // Custom Flight API
  Status
  DuckDBService::DoPlanPushdown( const ServerCallContext&  context
                                ,const shared_ptr<Buffer>  plan_data
                                ,unique_ptr<ResultStream>* result) {
    static uint32_t PushdownID { 0 };

    // >> Parse phase
    SkytetherDebugMsg("Starting parse phase");
    SkytetherStartTS(DuckService);

    SkytetherStartTS(ParsePhase);
    unique_ptr<SystemPlan> sys_plan = mohair::SystemPlanFrom(
      mohair::PlanMessage::FromString(plan_data->ToString())
    );

    SkytetherDebugMsg("Parsed pushdown plan:");
    mohair::PrintSubstraitPlan(*(sys_plan->plan_msg->payload));

    string plan_id {
        sys_plan->plan_msg->payload->plan_id()
      + std::to_string(++PushdownID)
    };

    // TODO: operator_id is in RelCommon
    const RelCommon& root_common = mohair::GetRelCommon(
      sys_plan->plan_msg->payload
                        ->relations(0)
                         .root()
                         .input()
    );

    uint32_t id_rootop = root_common.operator_id();

    SkytetherStopTS(ParsePhase);
    SkytetherTrackDecomposeTS(
       ParsePhase
      ,DecomposeStats::PARSE
      ,sys_plan->plan_msg->payload
      ,id_rootop
    );

    // >> Delegation phase
    SkytetherDebugMsg("Starting delegation phase");

    SkytetherStartTS(DelegatePhase);

    auto result_decomposed = CoopDecomposePlan(std::move(sys_plan), context.peer());
    if (not result_decomposed.ok()) {
      PrintError("Failed during decomposition", result_decomposed.status());
      return result_decomposed.status();
    }

    auto [pushback, exec_sysplan, ctx_id, view_id] = (
      std::move(result_decomposed).ValueOrDie()
    );

    SkytetherStopTS(DelegatePhase);

    // Send pushback plan upstream first
    SkytetherDebugMsg("Sending Pushback");
    *result = std::make_unique<SimpleResultStream>(
      vector<FlightResult> { PushbackResult { std::move(pushback) } }
    );

    // TODO: update this portion by replacing ProcessForExecution
    // >> Execution phase
    //    NOTE: Sending pushback before finishing query execution affects recovery
    SkytetherDebugMsg("Starting execution phase");
    SkytetherStartTS(ExecutePhase);

    // Begin query execution (concurrent with upstream processing of pushback)
    try { ARROW_RETURN_NOT_OK(engine->ExecuteContext(ctx_id, view_id)); }
    catch (const std::exception& duck_err) {
      SkytetherDebugMsg("DuckDB exception: " << duck_err.what());
      return Status::Invalid("DuckDB execution failed");
    }

    SkytetherStopTS(ExecutePhase);
    SkytetherStopTS(DuckService);

    SkytetherLogLabeledTimestamps(plan_id, ParsePhase);
    SkytetherLogLabeledTimestamps(plan_id, DelegatePhase);
    SkytetherLogLabeledTimestamps(plan_id, ExecutePhase);
    SkytetherLogLabeledTimestamps(plan_id, DuckService);

    SkytetherDebugMsg("Query processing complete");

    return Status::OK();
  }

  Status
  DuckDBService::ClearViews( [[maybe_unused]] const ServerCallContext&  context
                            ,[[maybe_unused]] const shared_ptr<Buffer>  plan_msg
                            ,[[maybe_unused]] unique_ptr<ResultStream>* result) {
    return Status::NotImplemented("TODO");
  }

  Status
  DuckDBService::DoPlanExecution( [[maybe_unused]] const ServerCallContext&  context
                                 ,[[maybe_unused]] const shared_ptr<Buffer>  plan_data
                                 ,[[maybe_unused]] unique_ptr<ResultStream>* result) {
    return Status::NotImplemented("TODO: query service");
  }


  // Standard Flight API
  Status
  DuckDBService::DoGet( [[maybe_unused]] const ServerCallContext&      context
                       ,                 const Ticket&                 request
                       ,                 unique_ptr<FlightDataStream>* result_stream) {
    SkyResultRel result_ticket;
    result_ticket.ParseFromString(request.ticket);
    const string& view_name = result_ticket.result_name();

    SkytetherDebugMsg(
         "Received request for ["
      << result_ticket.context_id() << " >> '"
      << view_name
      << "']"
    );
    DuckContext* ctx     = engine->GetDuckContext(result_ticket.context_id());
    QueryStatus  qstatus = ctx->status.load();
    if (qstatus != QueryStatus::Complete) {
      unique_lock<mutex> status_lock(ctx->status_mtx);
      ctx->status_cv.wait(status_lock
        ,[ctx]() { return ctx->status.load() == QueryStatus::Complete; }
      );
    }
    ARROW_ASSIGN_OR_RAISE(auto resultset_reader, engine->ScanResults(view_name));

    SkytetherDebugMsg("Returning results for [" << view_name << "]");
    *result_stream = std::make_unique<RecordBatchStream>(resultset_reader);
    return Status::OK();
  }


  // >> Support methods
  bool IsResultRel(const Rel* rel) {
    if (not rel->has_extension_leaf()) { return false; }
    return rel->extension_leaf().detail().Is<SkyResultRel>();
  }

  vector<Rel*>
  FindResultRels(mohair::MohairOp* rel, vector<Rel*>& result_vec) {
    // Recurse
    for (size_t op_ndx = 0; op_ndx < rel->GetOpArity(); ++op_ndx) {
      FindResultRels(rel->GetOpInputs()[op_ndx].get(), result_vec);
    }

    // Add ourself last
    if (IsResultRel(rel->substrait_rel)) {
      result_vec.push_back(rel->substrait_rel);
    }

    return result_vec;
  }

  vector<Rel*> FindResultRels(Plan& src_plan) {
    unique_ptr<mohair::MohairOp> root_op = mohair::MohairFrom(&src_plan);

    // Recurse
    vector<Rel*> result_vec;
    for (size_t op_ndx = 0; op_ndx < root_op->GetOpArity(); ++op_ndx) {
      FindResultRels(root_op->GetOpInputs()[op_ndx].get(), result_vec);
    }

    // Add the root last
    if (IsResultRel(root_op->substrait_rel)) {
      result_vec.push_back(root_op->substrait_rel);
    }

    return result_vec;
  }

  Status
  MaterializeLocally(SkytetherClient& conn, Plan& pushback, EngineDuckDB& engine) {
    // Find all Rels containing the pushback information
    vector<Rel*> result_rels = FindResultRels(pushback);

    for (Rel* result_rel : result_rels) {
      // Pull the results for the pushdown (described in the pushback)
      SkytetherTicket result_ticket = SkytetherTicket::FromRel(*result_rel);
      SkytetherDebugMsg("Requesting pushdown results (" << result_ticket.Name() << ")");
      ARROW_ASSIGN_OR_RAISE(
         RecordBatchVector result_batches
        ,RequestResultSet(conn, result_ticket)
      );

      ARROW_RETURN_NOT_OK(engine.MaterializeResults(result_ticket.Name(), result_batches));
    }

    return Status::OK();
  }

  // TODO: make async
  //! Delegates a pushdown plan to a downstream connection
  Result<unique_ptr<PlanMessage>>
  SendPushdown(SkytetherClient& conn, PlanMessage& pushdown_msg, EngineDuckDB& engine) {
    /* DEBUGGING
    SkytetherDebugMsg("Sending Pushdown");
    mohair::PrintSubstraitPlan(*(pushdown_msg.payload));
    */

    // Clear the stats so they don't get duplicated on the pushback path
    pushdown_msg.payload->clear_decompose_stats();

    // Send the pushdown message and store the pushback response
    SkytetherStartTS(ActionServiceSendPushdown);
    ARROW_ASSIGN_OR_RAISE(unique_ptr<Plan> pushback, conn.SendQueryPlan(pushdown_msg));
    SkytetherStopTS(ActionServiceSendPushdown);

    /* DEBUGGING
    SkytetherDebugMsg("Received Pushback");
    mohair::PrintSubstraitPlan(*pushback);
    */

    // TODO: enable streaming results instead of materializing
    // Materialize the SkyResultRel in the pushback response
    SkytetherStartTS(ActionServiceMaterialize);
    ARROW_RETURN_NOT_OK(MaterializeLocally(conn, *pushback, engine));
    SkytetherStopTS(ActionServiceMaterialize);

    SkytetherLogTimestamps(ActionServiceSendPushdown);
    SkytetherLogTimestamps(ActionServiceMaterialize);

    // Return the parsed pushback plan
    return mohair::PlanMessage::FromPlan(std::move(pushback));
  }

  //! Logic for eager cooperative decomposition of a query plan.
  //  Eager splitting occurs before pushdown so that only subplans are delegated.
  Result<unique_ptr<SystemPlan>>
  DuckDBService::EagerDecomposeDelegate(unique_ptr<SystemPlan> sys_plan) {
    // Split, if possible, otherwise the pushdown is the whole plan
    SkytetherStartTS(EagerSplitStep);

    unique_ptr<PlanSplit> eager_split {
      PlanSplit::FindSplit(sys_plan.get(), this->service_cfg->decompose_alg())
    };

    eager_split->PrintSplit();

    uint32_t id_splitrel { 0 };
    vector<unique_ptr<PlanMessage>> pushdown_msgs;

    if (not eager_split->CanSplit()) {
      pushdown_msgs.push_back(std::move(sys_plan->plan_msg));
    }

    else {
      // Get the operator ID of the stage sink
      const RelCommon& mergerel_common = mohair::GetRelCommon(
        *(eager_split->stage->sink->substrait_rel)
      );

      id_splitrel = mergerel_common.operator_id();
      pushdown_msgs = eager_split->ExtractSubplans();
    }

    SkytetherStopTS(EagerSplitStep);
    SkytetherTrackDecomposeTS(
       EagerSplitStep
      ,DecomposeStats::SPLIT
      ,eager_split->super_plan
      ,id_splitrel
    );

    // TODO: this should be async
    // For each downstream service, delegate and materialize
    for (size_t conn_ndx = 0; conn_ndx < service_conns.size(); ++conn_ndx) {
      /* DEBUGGING
      SkytetherDebugMsg("Sending requests to connection [" << std::to_string(conn_ndx) << "]");
      */

      SkytetherClient& cse_conn = *(service_conns[conn_ndx]);

      if (eager_split->CanSplit()) {
        for (OpPipeline* opipe : eager_split->RemainingOrigins()) {
          SkytetherStartTS(SendOriginRequestsStep);
          unique_ptr<PlanMessage> origin_msg = eager_split->ExtractOriginMessage(opipe);

          // Send the pushdown message and materialize SkyResultRel ops
          ARROW_ASSIGN_OR_RAISE(
             unique_ptr<PlanMessage> origin_pushback
            ,SendPushdown(cse_conn, *origin_msg, *engine)
          );

          // Propagate the runtime stats from the pushback
          auto pushback_stats = origin_pushback->payload->decompose_stats();
          for (auto dstat : pushback_stats) {
            auto upstream_dstat = sys_plan->plan_msg->payload->add_decompose_stats();
            upstream_dstat->CopyFrom(dstat);
          }

          // Find each result rel (they should be materialized already)
          vector<Rel*> origin_resultrels = FindResultRels(*(origin_pushback->payload));
          if (origin_resultrels.size() != 1) {
            return Status::Invalid("Expected a single ResultRel for origin request");
          }

          // replace the origin request with the materialized view to read from
          if (not eager_split->MergeOriginResult(opipe, origin_resultrels[0])) {
            return Status::Invalid("Unable to merge origin result into super plan");
          }

          SkytetherStopTS(SendOriginRequestsStep);
          SkytetherTrackDecomposeTS(
             SendOriginRequestsStep
            ,DecomposeStats::ORIGIN
            ,sys_plan->plan_msg->payload
            ,mohair::GetRelCommon(opipe->sink->substrait_rel)->operator_id()
          );
        }
      }

      for (size_t msg_ndx = 0; msg_ndx < pushdown_msgs.size(); ++msg_ndx) {
        PlanMessage&     pushdown_msg    = *(pushdown_msgs[msg_ndx]);
        const RelCommon& pushdown_common = mohair::GetRelCommon(
          pushdown_msg.payload->relations(0).root().input()
        );
        uint32_t id_pushdown = pushdown_common.operator_id();

        // Send the pushdown message and materialize SkyResultRel ops
        SkytetherStartTS(EagerDelegateStep);
        ARROW_ASSIGN_OR_RAISE(
           unique_ptr<PlanMessage> pushback_msg
          ,SendPushdown(cse_conn, pushdown_msg, *engine)
        );

        SkytetherStopTS(EagerDelegateStep);

        // >> Merge the pushback plan(s)
        // TODO: allow for a SkyResultRel to coalesce from many CSEs

        // Special case if we delegated whole plan
        if (not eager_split->CanSplit()) {
          SkytetherStartTS(EagerMergeStep);
          sys_plan = mohair::SystemPlanFrom(std::move(pushback_msg));

          SkytetherStopTS(EagerMergeStep);

          SkytetherTrackDecomposeTS(
             EagerDelegateStep
            ,DecomposeStats::DELEGATE
            ,sys_plan->plan_msg->payload
            ,id_pushdown
          );
          SkytetherTrackDecomposeTS(
             EagerMergeStep
            ,DecomposeStats::MERGE
            ,sys_plan->plan_msg->payload
            ,id_pushdown
          );
        }

        // If we need to merge pushback for a delegated subplan
        else {
          // Propagate the runtime stats from the pushback
          auto pushback_stats = pushback_msg->payload->decompose_stats();
          for (auto downstream_dstat : pushback_stats) {
            auto upstream_dstat = eager_split->super_plan->add_decompose_stats();
            upstream_dstat->CopyFrom(downstream_dstat);
          }

          // Merge the pushback plan
          SkytetherStartTS(EagerMergeStep);
          eager_split->MergeSubplan(pushback_msg.get());

          SkytetherStopTS(EagerMergeStep);

          SkytetherTrackDecomposeTS(
             EagerDelegateStep
            ,DecomposeStats::DELEGATE
            ,eager_split->super_plan
            ,id_pushdown
          );
          SkytetherTrackDecomposeTS(
             EagerMergeStep
            ,DecomposeStats::MERGE
            ,eager_split->super_plan
            ,id_pushdown
          );
        }
      }
    }

    return sys_plan;
  }

  //! Logic for lazy cooperative decomposition of a query plan.
  //  Lazy splitting occurs after pushback is received, so the entire plan is delegated.
  Result<unique_ptr<SystemPlan>>
  DuckDBService::DelegatePushdown(unique_ptr<SystemPlan> sys_plan) {
    unique_ptr<PlanMessage> pushback_msg { nullptr };

    // TODO: add capability to merge many pushback messages
    constexpr size_t count_conns = 1;
    if (service_conns.size() > count_conns) {
      SkytetherDebugMsg(
        "Unable to delegate pushdown to [" << service_conns.size() << "] services."
      );
    }

    // TODO: this should be async
    // Delegate the whole plan to each downstream service; merge each pushback plan
    for (size_t conn_ndx = 0; conn_ndx < count_conns; ++conn_ndx) {
      SkytetherClient& cse_conn     = *(service_conns[conn_ndx]);
      PlanMessage&     pushdown_msg = *(sys_plan->plan_msg);

      const RelCommon& pd_common = mohair::GetRelCommon(
        pushdown_msg.payload->relations(0).root().input()
      );
      uint32_t id_pushdown = pd_common.operator_id();

      // Send the pushdown message and materialize SkyResultRel ops
      SkytetherStartTS(LazyDelegateStep);
      ARROW_ASSIGN_OR_RAISE(pushback_msg, SendPushdown(cse_conn, pushdown_msg, *engine));

      SkytetherStopTS(LazyDelegateStep);
      SkytetherTrackDecomposeTS(
         LazyDelegateStep
        ,DecomposeStats::DELEGATE
        ,pushback_msg->payload
        ,id_pushdown
      );
    }

    return mohair::SystemPlanFrom(std::move(pushback_msg));
  }

  Result<std::tuple<unique_ptr<Plan>, size_t, uint64_t>>
  DuckDBService::EagerDecomposeTranslate(SystemPlan& exec_sysplan, const string& loc) {
    unique_ptr<Plan> pushback_plan;
    size_t           ctx_id;
    uint64_t         view_id;

    // Eager decomposition tries to execute the whole plan; only pushback the result
    try {
      // Create an execution context with the translated plan
      std::tie(ctx_id, view_id) = engine->CreateExecutionContext(exec_sysplan);

      // Create the pushback plan from untranslated portion of the system plan
      pushback_plan = PushbackFromExecutionPlan(
         exec_sysplan
        ,ResultRelWithContext(
           exec_sysplan.plan_msg->payload.get(), ctx_id, view_id, loc
         )
      );
    }
    catch (const std::exception& duck_err) {
      SkytetherDebugMsg("DuckDB exception: " << duck_err.what());
      return Status::Invalid("DuckDB execution failed");
    }

    return std::make_tuple(std::move(pushback_plan), ctx_id, view_id);
  }

  Result<std::tuple<unique_ptr<Plan>, unique_ptr<SystemPlan>, size_t, uint64_t>>
  DuckDBService::LazyDecomposeTranslate( unique_ptr<SystemPlan> pushback_sysplan
                                        ,const string&          loc) {
    unique_ptr<Plan>       pushback_plan;
    unique_ptr<SystemPlan> exec_sysplan;
    size_t                 ctx_id;
    uint64_t               view_id;

    // Lazy decomposition decomposes the pushback; only execute a subplan
    SkytetherStartTS(LazySplitStep);

    unique_ptr<PlanSplit> lazy_splitter {
      PlanSplit::FindSplit(pushback_sysplan.get(), DecomposeAlg::LongPipelineLeaf)
    };

    lazy_splitter->PrintSplit();

    // Determine what to use as the execution plan
    if (not lazy_splitter->CanSplit()) { exec_sysplan = std::move(pushback_sysplan); }

    // Only if we can split will there be a timing for SPLIT
    else {
      exec_sysplan = mohair::SystemPlanFrom(lazy_splitter->ExtractExecSubplan());

      const RelCommon& mergerel_common = mohair::GetRelCommon(
        *(lazy_splitter->stage->sink->substrait_rel)
      );
      uint32_t id_splitrel = mergerel_common.operator_id();

      SkytetherStopTS(LazySplitStep);
      SkytetherTrackDecomposeTS(
         LazySplitStep
        ,DecomposeStats::SPLIT
        ,lazy_splitter->super_plan
        ,id_splitrel
      );
    }

    try {
      SkytetherStartTS(LazyTranslateStep);

      const RelCommon& execplan_root = mohair::GetRelCommon(
        exec_sysplan->plan_msg->payload->relations(0).root().input()
      );
      uint32_t id_execroot = execplan_root.operator_id();

      // Create an execution context with the translated plan
      std::tie(ctx_id, view_id) = engine->CreateExecutionContext(*exec_sysplan);

      // Create the pushback plan from untranslated portion of the system plan
      unique_ptr<Rel> result_rel = ResultRelWithContext(
        exec_sysplan->plan_msg->payload.get(), ctx_id, view_id, loc
      );

      if (not lazy_splitter->CanSplit()) {
        pushback_plan = PushbackFromExecutionPlan(*exec_sysplan, std::move(result_rel));
      }
      else {
        // Undo split annotations by replacing the subplan with a SkyResultRel
        lazy_splitter->MergeResultRel(result_rel.get());
        pushback_plan = std::move(pushback_sysplan->plan_msg->payload);
      }

      SkytetherStopTS(LazyTranslateStep);
      SkytetherTrackDecomposeTS(
         LazyTranslateStep
        ,DecomposeStats::TRANSLATE
        ,pushback_plan
        ,id_execroot
      );
    }
    catch (const std::exception& duck_err) {
      SkytetherDebugMsg("DuckDB exception: " << duck_err.what());
      return Status::Invalid("DuckDB execution failed");
    }

    return std::make_tuple(
       std::move(pushback_plan), std::move(exec_sysplan), ctx_id, view_id
    );
  }


  //! Entry point into cooperative decomposition logic.
  Result<std::tuple<unique_ptr<Plan>, unique_ptr<SystemPlan>, size_t, uint64_t>>
  DuckDBService::CoopDecomposePlan(unique_ptr<SystemPlan> sys_plan, const string& loc) {
    unique_ptr<Plan>       pushback_plan;
    unique_ptr<SystemPlan> exec_sysplan;
    size_t                 ctx_id;
    uint64_t               view_id;

    ARROW_ASSIGN_OR_RAISE(
       bool should_eagersplit, ShouldDecomposeEagerly(service_cfg->decompose_alg())
    );

    // >> Eager cooperative decomposition ("steal" operators)
    if (should_eagersplit) {
      // >> Delegate pushdown plans
      // a leaf engine uses the whole plan as the execution plan
      if (service_conns.empty()) {
        exec_sysplan = std::move(sys_plan);
      }

      // non-leaf engines decomposes the system plan and delegates pushdown plans
      else {
        ARROW_ASSIGN_OR_RAISE(
           exec_sysplan
          ,EagerDecomposeDelegate(std::move(sys_plan))
        );
      }

      const PlanRel& exec_planrel = exec_sysplan->plan_msg->payload->relations(0);
      if (not exec_planrel.has_root()) {
        return Status::Invalid("Expected first exec PlanRel to be root subtree");
      }

      const RelCommon& execplan_root = mohair::GetRelCommon(exec_planrel.root().input());
      uint32_t id_execroot = execplan_root.operator_id();

      // >> Translate execution plan
      SkytetherStartTS(EagerTranslateStep);
      auto result_eagerexec = this->EagerDecomposeTranslate(*exec_sysplan, loc);
      SkytetherStopTS(EagerTranslateStep);

      if (not result_eagerexec.ok()) {
        PrintError("Failed during execution", result_eagerexec.status());
        return result_eagerexec.status();
      }

      std::tie(pushback_plan, ctx_id, view_id) = std::move(result_eagerexec).ValueOrDie();
      size_t count_tablebytes = engine->EstimateTotalTableSize();

      bool only_materialize = IsResultRel(&(exec_planrel.root().input()));
      SkytetherTrackDecomposeTS(
         EagerTranslateStep
        ,only_materialize ? DecomposeStats::MATERIALIZE : DecomposeStats::TRANSLATE
        ,pushback_plan
        ,id_execroot
      );
    }

    // >> Lazy cooperative decomposition ("reactive")
    else {
      // >> Delegate system plan
      unique_ptr<SystemPlan> pb_sysplan;

      // a leaf engine uses the system plan as the pushback plan
      if (service_conns.empty()) { pb_sysplan = std::move(sys_plan); }

      // non-leaf engines delegate the system plan then merge into a single pushback plan
      else { ARROW_ASSIGN_OR_RAISE(pb_sysplan, DelegatePushdown(std::move(sys_plan))); }

      // >> Decompose execution plan, then translate it
      auto   result_lazyexec  = this->LazyDecomposeTranslate(std::move(pb_sysplan), loc);
      size_t count_tablebytes = engine->EstimateTotalTableSize();

      if (not result_lazyexec.ok()) {
        PrintError("Failed during execution", result_lazyexec.status());
        return result_lazyexec.status();
      }

      std::tie(pushback_plan, exec_sysplan, ctx_id, view_id) = (
        std::move(result_lazyexec).ValueOrDie()
      );
    }

    return std::make_tuple(
      std::move(pushback_plan), std::move(exec_sysplan), ctx_id, view_id
    );
  }

} // namespace: skytether::services

