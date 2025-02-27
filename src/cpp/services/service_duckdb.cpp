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
  ResultRelWithContext(Plan* view_plan, size_t ctx_id, const string& name, const string& loc) {
    // Initialize SkyResultRel
    unique_ptr<SkyResultRel> skyrel = mohair::CreateResultRelForPlan(view_plan);
    skyrel->set_context_id(ctx_id);
    skyrel->set_result_name(name);
    skyrel->set_service_location(loc);

    // Initialize ExtensionLeafRel inside a Rel
    auto result_rel = std::make_unique<Rel>();
    result_rel->set_allocated_extension_leaf(new ExtensionLeafRel);

    // Then, initialize `ExtensionLeafRel`
    ExtensionLeafRel* leaf_rel = result_rel->mutable_extension_leaf();
    leaf_rel->mutable_common()
            ->mutable_hint()
            ->mutable_output_schema()
            ->CopyFrom(skyrel->schema());
    leaf_rel->mutable_detail()->PackFrom(*skyrel);

    return result_rel;
  }

  unique_ptr<Plan>
  PushbackFromExecutionPlan(const SystemPlan& exec_plan, unique_ptr<Rel> result_rel) {
    // Initialize pushbaack from execution plan
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
    string srv_id = EngineIDForLocation(this->service_cfg->service_location());
    engine = skytether::engines::DuckDBForFile(srv_id, db_fpath);
  }

  DuckDBService::DuckDBService( unique_ptr<ServiceConfig>&& cfg
                               ,ShutdownCallback*           cb_custom)
    : EngineService(std::move(cfg), cb_custom) {
    string srv_id = EngineIDForLocation(this->service_cfg->service_location());
    engine = skytether::engines::DuckDBForMem(srv_id);
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
    // >> Parse phase
    SkytetherDebugMsg("Starting parse phase");
    unique_ptr<SystemPlan> sys_plan;

    SkytetherLogPerf(DuckServicePhaseParse,
      {
        sys_plan = mohair::SystemPlanFrom(
          mohair::PlanMessage::FromString(plan_data->ToString())
        );
      }
    );

    // >> Delegation phase
    SkytetherDebugMsg("Starting delegation phase");
    SkytetherStartTS(DuckServicePhaseDelegation);

    auto result_decomposed = CoopDecomposePlan(std::move(sys_plan), context.peer());
    if (not result_decomposed.ok()) {
      PrintError("Failed during decomposition", result_decomposed.status());

      SkytetherStopTS(DuckServicePhaseDelegation);
      SkytetherLogTimestamps(DuckServicePhaseDelegation);

      return result_decomposed.status();
    }

    auto [pushback, exec_sysplan, ctx_id, view_name] = (
      std::move(result_decomposed).ValueOrDie()
    );

    // Send pushback plan upstream first
    SkytetherDebugMsg("Sending pushback");
    *result = std::make_unique<SimpleResultStream>(
      vector<FlightResult> { PushbackResult { std::move(pushback) } }
    );

    SkytetherStopTS(DuckServicePhaseDelegation);
    SkytetherLogTimestamps(DuckServicePhaseDelegation);

    // TODO: update this portion by replacing ProcessForExecution
    // >> Execution phase
    //    NOTE: Sending pushback before finishing query execution affects recovery
    SkytetherDebugMsg("Starting execution phase");
    SkytetherStartTS(DuckServicePhaseExecution);

    // Begin query execution (concurrent with upstream processing of pushback)
    try { ARROW_RETURN_NOT_OK(engine->ExecuteContext(ctx_id, view_name)); }
    catch (const std::exception& duck_err) {
      SkytetherDebugMsg("DuckDB exception: " << duck_err.what());
      return Status::Invalid("DuckDB execution failed");
    }

    SkytetherStopTS(DuckServicePhaseExecution);
    SkytetherLogTimestamps(DuckServicePhaseExecution);

    SkytetherDebugMsg("Query processing complete");
    return Status::OK();
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
  bool IsResultRel(Rel* rel) {
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
      SkytetherDebugMsg("Found result rel: ");
      mohair::PrintSubstraitRel(rel->substrait_rel);
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
    SkytetherDebugMsg("Locally materializing subplan results");

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
    unique_ptr<Plan> pushback;

    // Send the pushdown message and store the pushback response
    SkytetherLogPerf(DuckServiceDelegateSubplan,
      { ARROW_ASSIGN_OR_RAISE(pushback, conn.SendQueryPlan(pushdown_msg)); }
    );

    // TODO: enable streaming results instead of materializing
    // Materialize the SkyResultRel in the pushback response
    SkytetherLogPerf(DuckServiceMaterializePushback,
      { ARROW_RETURN_NOT_OK(MaterializeLocally(conn, *pushback, engine)); }
    );

    // Return the parsed pushback plan
    return mohair::PlanMessage::FromPlan(std::move(pushback));
  }

  //! Logic for lazy cooperative decomposition of a query plan.
  //  Lazy splitting occurs after pushback is received, so the entire plan is delegated.
  Result<unique_ptr<SystemPlan>>
  DuckDBService::DelegatePushdown(unique_ptr<SystemPlan> sys_plan) {
    unique_ptr<PlanMessage> pushback_plan;

    // TODO: this should be async
    // Delegate the whole plan to each downstream service; merge each pushback plan
    for (size_t conn_ndx = 0; conn_ndx < service_conns.size(); ++conn_ndx) {
      SkytetherClient& cse_conn     = *(service_conns[conn_ndx]);
      PlanMessage&     pushdown_msg = *(sys_plan->plan_msg);

      // Send the pushdown message and materialize SkyResultRel ops
      ARROW_ASSIGN_OR_RAISE(
         unique_ptr<PlanMessage> pushback_msg
        ,SendPushdown(cse_conn, pushdown_msg, *engine)
      );

      // TODO: add capability to merge many pushback messages
      if (pushback_plan != nullptr) {
        return Status::Invalid("Not yet supported: merging many pushback plans");
      }

      pushback_plan = std::move(pushback_msg); 
    }

    return mohair::SystemPlanFrom(std::move(pushback_plan));
  }

  //! Logic for eager cooperative decomposition of a query plan.
  //  Eager splitting occurs before pushdown so that only subplans are delegated.
  Result<unique_ptr<SystemPlan>>
  DuckDBService::EagerDecomposeDelegate(unique_ptr<SystemPlan> sys_plan) {
    unique_ptr<PlanSplit> eager_split {
      PlanSplit::FindSplit(sys_plan.get(), this->service_cfg->decompose_alg())
    };

    // Split, if possible, otherwise the pushdown is the whole plan
    vector<unique_ptr<PlanMessage>> pushdown_msgs;
    if (eager_split->CanSplit()) { pushdown_msgs = eager_split->ExtractSubplans(); }
    else { pushdown_msgs.push_back(std::move(sys_plan->plan_msg)); }

    // TODO: this should be async
    // For each downstream service, delegate and materialize
    for (size_t conn_ndx = 0; conn_ndx < service_conns.size(); ++conn_ndx) {
      SkytetherClient& cse_conn = *(service_conns[conn_ndx]);

      for (size_t msg_ndx = 0; msg_ndx < pushdown_msgs.size(); ++msg_ndx) {
        PlanMessage& pushdown_msg = *(pushdown_msgs[msg_ndx]);

        // Send the pushdown message and materialize SkyResultRel ops
        ARROW_ASSIGN_OR_RAISE(
           unique_ptr<PlanMessage> pushback_msg
          ,SendPushdown(cse_conn, pushdown_msg, *engine)
        );

        // TODO: allow for a SkyResultRel to coalesce from many CSEs
        // Merge the pushback plan
        if (eager_split->CanSplit()) { eager_split->MergeSubplan(pushback_msg.get()); }
        else            { sys_plan = mohair::SystemPlanFrom(std::move(pushback_msg)); }
      }
    }

    return sys_plan;
  }


  //! Entry point into cooperative decomposition logic.
  Result<std::tuple<unique_ptr<Plan>, unique_ptr<SystemPlan>, size_t, string>>
  DuckDBService::CoopDecomposePlan(unique_ptr<SystemPlan> sys_plan, const string& loc) {
    unique_ptr<Plan>       pushback_plan;
    unique_ptr<SystemPlan> exec_sysplan;
    size_t                 ctx_id;
    string                 view_name;

    SkytetherStartTS(DuckServiceDecomposeDelegate);

    ARROW_ASSIGN_OR_RAISE(
       bool should_eagersplit, ShouldDecomposeEagerly(service_cfg->decompose_alg())
    );

    if (should_eagersplit) {
      // If we are the leaf device, the whole plan is our execution plan
      if (service_conns.empty()) {
        exec_sysplan = std::move(sys_plan);
      }
      else {
        ARROW_ASSIGN_OR_RAISE(
           exec_sysplan
          ,EagerDecomposeDelegate(std::move(sys_plan))
        );
      }

      // Eager decomposition tries to execute the whole plan; only pushback the result
      try {
        std::tie(ctx_id, view_name) = engine->CreateExecutionContext(*exec_sysplan);
        unique_ptr<Rel> result_rel  = ResultRelWithContext(
          exec_sysplan->plan_msg->payload.get(), ctx_id, view_name, loc
        );

        pushback_plan = PushbackFromExecutionPlan(*exec_sysplan, std::move(result_rel));
      }
      catch (const std::exception& duck_err) {
        SkytetherDebugMsg("DuckDB exception: " << duck_err.what());
        return Status::Invalid("DuckDB execution failed");
      }
    }

    else {

      // If we are the leaf device, the whole plan is like a pushback
      unique_ptr<SystemPlan> pushback_sysplan;
      if (service_conns.empty()) {
        pushback_sysplan = std::move(sys_plan);
      }
      else {
        ARROW_ASSIGN_OR_RAISE(
           pushback_sysplan
          ,DelegatePushdown(std::move(sys_plan))
        );
      }

      // Lazy decomposition decomposes the pushback; only execute a subplan
      unique_ptr<PlanSplit> lazy_splitter {
        PlanSplit::FindSplit(pushback_sysplan.get(), DecomposeAlg::LongPipelineLeaf)
      };

      // Determine what to use as the execution plan
      if (not lazy_splitter->CanSplit())   { exec_sysplan = std::move(pushback_sysplan); }
      else { exec_sysplan = mohair::SystemPlanFrom(lazy_splitter->ExtractExecSubplan()); }

      // Create execution context and construct the pushback plan
      try {
        std::tie(ctx_id, view_name) = engine->CreateExecutionContext(*exec_sysplan);
        unique_ptr<Rel> result_rel  = ResultRelWithContext(
          exec_sysplan->plan_msg->payload.get(), ctx_id, view_name, loc
        );

        if (not lazy_splitter->CanSplit()) {
          pushback_plan = PushbackFromExecutionPlan(*exec_sysplan, std::move(result_rel));
        }
        if (lazy_splitter->CanSplit()) {
          // Undo split annotations by replacing the subplan with a SkyResultRel
          lazy_splitter->MergeResultRel(result_rel.get());
          pushback_plan = std::move(pushback_sysplan->plan_msg->payload);
        }
      }
      catch (const std::exception& duck_err) {
        SkytetherDebugMsg("DuckDB exception: " << duck_err.what());
        return Status::Invalid("DuckDB execution failed");
      }
    }

    SkytetherStopTS(DuckServiceDecomposeDelegate);
    SkytetherLogTimestamps(DuckServiceDecomposeDelegate);

    return std::make_tuple(
       std::move(pushback_plan)
      ,std::move(exec_sysplan)
      ,ctx_id
      ,view_name
    );
  }

} // namespace: skytether::services

