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
  DuckDBService::DoPlanPushdown( [[maybe_unused]] const ServerCallContext&  context
                                ,                 const shared_ptr<Buffer>  plan_data
                                ,                 unique_ptr<ResultStream>* result) {
    // >> Parse phase
    SkytetherDebugMsg("Starting parse phase");
    unique_ptr<SystemPlan> sys_plan {
      mohair::SystemPlanFrom(
        mohair::PlanMessage::FromString(plan_data->ToString())
      )
    };

    // >> Delegation phase
    if (not service_conns.empty()) {
      SkytetherDebugMsg("Starting delegation phase");
      ARROW_RETURN_NOT_OK(CoopDecomp(sys_plan));
    }

    // >> Execution phase
    //    NOTE: Sending pushback before finishing query execution affects recovery
    SkytetherDebugMsg("Starting execution phase");
    try {
      // Translate to execution plan and construct pushback plan
      auto [pushback, ctx_id, view_name] = engine->ProcessForExecution(
        *sys_plan, context.peer()
      );

      // Send pushback plan upstream first
      SkytetherDebugMsg("Sending pushback");
      *result = std::make_unique<SimpleResultStream>(
        vector<FlightResult> { PushbackResult { std::move(pushback) } }
      );

      // Begin query execution (concurrent with upstream processing of pushback)
      SkytetherDebugMsg("Executing query");
      ARROW_RETURN_NOT_OK(engine->ExecuteContext(ctx_id, view_name));
    }
    catch (const std::exception& duck_err) {
      SkytetherDebugMsg("DuckDB exception: " << duck_err.what());
      return Status::Invalid("DuckDB execution failed");
    }

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

  Status MaterializeLocally(SkytetherClient& conn, Plan& pushback, EngineDuckDB& engine) {
    SkytetherDebugMsg("Locally materializing subplan results");

    // Find the Rel containing the pushback information
    int        root_relndx = mohair::FindPlanRoot(pushback);
    const Rel& result_rel  = pushback.relations(root_relndx).root().input();
    if (not result_rel.has_extension_leaf()) {
      return Status::Invalid("Expected a simple pushback plan");
    }

    // Pull the results for the pushdown (described in the pushback)
    SkytetherTicket result_ticket = SkytetherTicket::FromRel(result_rel);
    SkytetherDebugMsg("Requesting pushdown results");
    ARROW_ASSIGN_OR_RAISE(
       RecordBatchVector result_batches
      ,RequestResultSet(conn, result_ticket)
    );

    ARROW_RETURN_NOT_OK(engine.MaterializeResults(result_ticket.Name(), result_batches));

    return Status::OK();
  }

  //! Delegates each subplan to each downstream connection
  //  TODO: this currently assumes only one connection can execute a subplan
  Status
  DuckDBService::DecomposePlan(SystemPlan& sys_plan, unique_ptr<PlanSplit> decomposer) {
    vector<unique_ptr<PlanMessage>> pushdown_msgs = decomposer->ExtractSubplans();

    for (size_t srv_ndx = 0; srv_ndx < service_conns.size(); ++srv_ndx) {
      SkytetherClient& conn = *(service_conns[srv_ndx]);

      // Propagate each pushdown plan
      for (size_t plan_ndx = 0; plan_ndx < pushdown_msgs.size(); ++plan_ndx) {
        PlanMessage& pushdown_msg = *(pushdown_msgs[plan_ndx]);

        // Send the subplan
        ARROW_ASSIGN_OR_RAISE(unique_ptr<Plan> pushback, conn.DelegatePlan(pushdown_msg));
        ARROW_RETURN_NOT_OK(MaterializeLocally(conn, *pushback, *engine));

        // Receive and merge the pushback plan
        auto pushback_msg { mohair::PlanMessage::FromPlan(std::move(pushback)) };
        decomposer->MergeSubplan(pushback_msg.get());
      }
    }

    return Status::OK();
  }

  //! Handles the process of cooperative decomposition.
  //  Currently, the plan is eagerly split. Then, plan(s) are delegated.
  //  TODO: eventually split the plan lazily
  Status
  DuckDBService::CoopDecomp(unique_ptr<SystemPlan>& sys_plan) {
    unique_ptr<PlanSplit> candidate_split {
      // PlanSplit::FindSplit(sys_plan.get(), DecomposeAlg::WideJoinHead)
      PlanSplit::FindSplit(sys_plan.get(), this->service_cfg->decompose_alg())
    };

    // Split, send subplans, then merge (back into sys_plan)
    if (candidate_split->CanSplit()) {
      return DecomposePlan(*sys_plan, std::move(candidate_split));
    }

    // Send the whole plan, then merge (replace whole sys_plan)
    // TODO: hardcoded to only send to first connection; requires a way to merge results
    //       from each connection
    SkytetherClient& conn = *(service_conns[0]);

    ARROW_ASSIGN_OR_RAISE(
       unique_ptr<Plan> pushback
      ,conn.DelegatePlan(*(sys_plan->plan_msg))
    );
    ARROW_RETURN_NOT_OK(MaterializeLocally(conn, *pushback, *engine));

    sys_plan = mohair::SystemPlanFrom(
      mohair::PlanMessage::FromPlan(std::move(pushback))
    );

    return Status::OK();
  }

} // namespace: skytether::services

