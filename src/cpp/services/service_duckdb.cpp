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

  // For generating test data
  static size_t plan_id { 1 };
  string UUIDGen(const string& path_prefix, const char* plan_type, size_t id, string tag=string{});

  string UUIDGen(const string& path_prefix, const char* plan_type, size_t id, string tag) {
    static const string path_suffix   { ".mohair" };
    const string        plan_basename { path_prefix + "--" + std::to_string(id) };

    if (not tag.empty()) {
      return plan_basename + "." + tag + "." + plan_type + path_suffix;
    }

    return plan_basename + "." + plan_type + path_suffix;
  }

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
    // >> Preprocessing phase
    // Internalize the received query plan
    unique_ptr<SystemPlan> sys_plan {
      mohair::SystemPlanFrom(
        mohair::PlanMessage::FromString(plan_data->ToString())
      )
    };

    // If we're not a leaf, split and delegate
    if (not service_conns.empty()) {
      SkytetherDebugMsg("Initiating cooperative decomposition");
      auto status_decomp = CoopDecomp(sys_plan);
      if (not status_decomp.ok()) {
        skytether::PrintError("Decomposition failed", status_decomp);
        return Status::Invalid("Failed to decompose plan for delegation");
      }

      SkytetherDebugMsg("Completed downstream processing");
    }

    // >> Execution phase
    // convert substrait plan to duckdb plan
    SkytetherDebugMsg("Processing system plan for execution");
    auto [pushback_msg, ctx_id, view_name] = engine->ProcessForExecution(
      *sys_plan, context.peer()
    );

    if (pushback_msg == nullptr) {
      SkytetherDebugMsg("Failed to process system plan");
      return Status::Invalid("Failed to execute plan [name: " + view_name + "]");
    }

    // Respond with pushback
    SkytetherDebugMsg("Responding with pushback");
    *result = std::make_unique<SimpleResultStream>(
      vector<FlightResult> { PushbackResult { std::move(pushback_msg->payload) } }
    );

    // execute the query and return the result (or OK)
    SkytetherDebugMsg("Creating view of pushdown plan");
    ARROW_RETURN_NOT_OK(engine->ExecuteContext(ctx_id, view_name));

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

    SkytetherDebugMsg("Received request for [" << view_name << "]");
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

  Status MaterializeResults(SkytetherClient& conn, Plan& pushback, EngineDuckDB& engine) {
    SkytetherDebugMsg("Creating result ticket");
    SkyResultRel result_rel;
    int        root_relndx = mohair::FindPlanRoot(pushback);
    const Rel& pb_root     = pushback.relations(root_relndx).root().input();
    if (not pb_root.has_extension_leaf()) {
      return Status::Invalid("Expected a simple pushback plan");
    }

    pb_root.extension_leaf().detail().UnpackTo(&result_rel);
    const string& view_name = result_rel.result_name();
    string ticket_data;
    result_rel.SerializeToString(&ticket_data);
    SkytetherTicket result_ticket { ticket_data };

    SkytetherDebugMsg("Requesting query results [" << view_name << "]");
    ARROW_ASSIGN_OR_RAISE(
       RecordBatchVector result_batches
      ,RequestResultSet(conn, result_ticket)
    );

    ARROW_RETURN_NOT_OK(engine.CreateView(view_name, result_batches));

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

        // Create test data for pushdown
        string server_plan_tag { std::to_string(srv_ndx) + "-" + std::to_string(plan_ndx) };
        string pushdown_testpath = UUIDGen(
           EngineIDForLocation(this->service_cfg->service_location())
          ,"pushdown"
          ,plan_id
          ,server_plan_tag
        );
        if (not pushdown_msg.SerializeToFile(pushdown_testpath.data())) {
          return Status::Invalid("Failed to write plan to file as test");
        }


        // Send the subplan
        ARROW_ASSIGN_OR_RAISE(unique_ptr<Plan> pushback, conn.DelegatePlan(pushdown_msg));

        ARROW_RETURN_NOT_OK(MaterializeResults(conn, *pushback, *engine));

        // Receive and merge the pushback plan
        auto pushback_msg { mohair::PlanMessage::FromPlan(std::move(pushback)) };

        // Create test data for pushback
        string pushback_testpath = UUIDGen(
           EngineIDForLocation(this->service_cfg->service_location())
          ,"pushback"
          ,plan_id
          ,server_plan_tag
        );
        if (not pushback_msg->SerializeToFile(pushback_testpath.data())) {
          return Status::Invalid("Failed to write plan to file as test");
        }

        decomposer->MergeSubplan(pushback_msg.get());

        // Create test data for merged plan
        string merged_testpath = UUIDGen(
           EngineIDForLocation(this->service_cfg->service_location())
          ,"merged"
          ,plan_id
          ,server_plan_tag
        );
        if (not decomposer->sys_plan->plan_msg->SerializeToFile(merged_testpath.data())) {
          return Status::Invalid("Failed to write plan to file as test");
        }
      }
      ++plan_id;
    }

    return Status::OK();
  }

  //! Handles the process of cooperative decomposition.
  //  Currently, the plan is eagerly split. Then, plan(s) are delegated.
  //  TODO: eventually split the plan lazily
  Status
  DuckDBService::CoopDecomp(unique_ptr<SystemPlan>& sys_plan) {
    unique_ptr<PlanSplit> split_widejoin {
      PlanSplit::FindSplit(sys_plan.get(), DecomposeAlg::WideJoinHead)
    };

    // Split, send subplans, then merge (back into sys_plan)
    if (split_widejoin->CanSplit()) {
      return DecomposePlan(*sys_plan, std::move(split_widejoin));
    }

    // Send the whole plan, then merge (replace whole sys_plan)
    // TODO: this is hardcoded to talk to only the first connection;
    //       this needs to resolve against connections
    // NOTE: probably send the same plan to each connection,
    //       then union the results from each
    string superplan_testpath = UUIDGen(
       EngineIDForLocation(this->service_cfg->service_location())
      ,"superplan"
      ,plan_id
    );
    if (not sys_plan->plan_msg->SerializeToFile(superplan_testpath.data())) {
      std::cerr << "Couldn't write to " << superplan_testpath << std::endl;
      return Status::Invalid("Failed to write plan to file as test");
    }

    ARROW_ASSIGN_OR_RAISE(
       unique_ptr<Plan> pushback
      ,service_conns[0]->DelegatePlan(*(sys_plan->plan_msg))
    );

    ARROW_RETURN_NOT_OK(MaterializeResults(*(service_conns[0]), *pushback, *engine));

    sys_plan = mohair::SystemPlanFrom(
      mohair::PlanMessage::FromPlan(std::move(pushback))
    );

    string pushback_testpath = UUIDGen(
       EngineIDForLocation(this->service_cfg->service_location())
      ,"pushback"
      ,plan_id++
    );

    if (not sys_plan->plan_msg->SerializeToFile(pushback_testpath.data())) {
      std::cerr << "Couldn't write to " << pushback_testpath << std::endl;
      return Status::Invalid("Failed to write plan to file as test");
    }

    return Status::OK();
  }

} // namespace: skytether::services

