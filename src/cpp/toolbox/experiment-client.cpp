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

// >> Standard libs
#include <unistd.h>

// >> Internal
#include "skytether_cli.hpp"
#include "services/types.hpp"
#include "services/service_skytether.hpp"


// ------------------------------
// Macros and Type Aliases

// >> Standard types
using std::unique_ptr;
using std::shared_ptr;

// >> Types
using skytether::Status;
using skytether::Buffer;

using skytether::services::Location;
using skytether::services::ResultStream;
using skytether::services::FlightStreamReader;

using skytether::services::FlightStreamChunk;

using skytether::services::SkytetherClient;
using skytether::services::SkytetherTicket;

using mohair::PlanMessage;
using mohair::Plan;

// >> Functions
using skytether::cli::ParseArgLocationUri;

// >> Mohair timing types
using skytether::SkytetherLogger;
using mohair::system_clock;
using mohair::steady_clock;
using mohair::SteadyTS;


// ------------------------------
// Structs and Classes

struct ClientActions {
  bool should_shutdown { false };
  bool should_dereg    { false };
  bool should_split    { false };
  bool should_notsplit { false };

  Location                service_loc;
  Location                target_loc;
  unique_ptr<PlanMessage> query_plan;


  // >> "Application Interface"
  //! Executes many queries to try every possible eager split.
  Status ExecuteManyQueries(SkytetherClient& client_conn) {
    SkytetherDebugMsg("Sending many query requests...");

    std::string plan_basename { "simple-experiment" };

    // Parse the source query plan
    auto sys_plan = mohair::SystemPlanFrom(std::move(query_plan));

    // Collect pointers to all sink operators
    std::vector<mohair::MohairOp*> sink_ops;
    for (mohair::MohairOp* plan_op : sys_plan->plan_ops) {
      if (plan_op->IsSink()) { sink_ops.push_back(plan_op); }
    }


    SkytetherStartTS(ClientTryAllEager);

    // Each query plan will be annotated with 2 splits
    size_t experiment_itr { 0 };
    for (size_t first_splitndx = 0; first_splitndx < sink_ops.size() - 1; ++first_splitndx) {

      size_t second_splitndx = first_splitndx + 1;
      for (; second_splitndx < sink_ops.size(); ++second_splitndx) {
        ++experiment_itr;

        // NOTE: set these every time because they get unset at split time
        sink_ops[first_splitndx]->substrait_rel->set_has_splitoverride(true);
        sink_ops[second_splitndx]->substrait_rel->set_has_splitoverride(true);

        std::string exp_id     { "." + std::to_string(experiment_itr) };
        std::string plan_name  { plan_basename + exp_id               };
        std::string plan_fpath { plan_name     + ".plan"              };

        // Assign an ID to plan that way we can distinguish between experiments
        sys_plan->plan_msg->payload->set_plan_id(plan_name);

        // Serialize annotated plan for debugging
        sys_plan->plan_msg->SerializeToFile(plan_fpath.data());

        // >> Send the query request and store the pushback response
        SkytetherStartTS(ClientExecuteQuery);
        ARROW_ASSIGN_OR_RAISE(
           unique_ptr<Plan> pushback_plan
          ,client_conn.SendQueryPlan(*(sys_plan->plan_msg))
        );
        SkytetherStopTS(ClientExecuteQuery);
        SkytetherLogTimestamps(ClientExecuteQuery);

        // >> Request the result set
        // TODO: hardcoded for now
        mohair::SkyResultRel result_rel;
        pushback_plan->relations(0).root()
                                   .input()
                                   .extension_leaf()
                                   .detail()
                                   .UnpackTo(&result_rel);

        SkytetherStartTS(ClientGetResultSet);
        SkytetherTicket result_ticket = SkytetherTicket::ForContext(
          result_rel.context_id(), result_rel.result_name()
        );

        ARROW_RETURN_NOT_OK(RequestResultSet(client_conn, result_ticket));
        SkytetherStopTS(ClientGetResultSet);
        SkytetherLogTimestamps(ClientGetResultSet);

        // >> Write pushback plan to file for analysis (contains latencies)
        std::string pushback_fpath {
          "experiment." + std::to_string(experiment_itr) + ".pushback"
        };

        auto pushback_msg = PlanMessage::FromPlan(std::move(pushback_plan));
        pushback_msg->SerializeToFile(pushback_fpath.data());

        // Cleanup second annotation
        sink_ops[second_splitndx]->substrait_rel->set_has_splitoverride(false);
      }

      // Cleanup first annotation
      sink_ops[first_splitndx]->substrait_rel->set_has_splitoverride(false);
    }

    SkytetherStopTS(ClientTryAllEager);
    SkytetherLogTimestamps(ClientTryAllEager);

    std::cout << "Completed all queries" << std::endl;
    return Status::OK();
  }

  //! Executes a query by submitting the query plan then fetching the results.
  Status ExecuteQuery(SkytetherClient& client_conn) {
    SkytetherDebugMsg("Sending query request");

    // Send the query request and store the pushback response
    ARROW_ASSIGN_OR_RAISE(
       unique_ptr<Plan> pushback_plan
      ,client_conn.SendQueryPlan(*query_plan)
    );

    // TODO: hardcoded for now
    mohair::SkyResultRel result_rel;
    pushback_plan->relations(0).root()
                               .input()
                               .extension_leaf()
                               .detail()
                               .UnpackTo(&result_rel);

    SkytetherDebugMsg(
         "Result name: " << result_rel.result_name()
      << " (ID: " << std::to_string(result_rel.context_id()) << ")"
    );

    // TODO: for now, going to retrieve from a different process
    // SkytetherTicket query_ticket { result_rel.context_id() };
    // ARROW_RETURN_NOT_OK(RequestResultSet(client_conn, query_ticket));

    std::cout << "Query complete" << std::endl;
    return Status::OK();
  }


  // >> Public entry point

  //! The entry point for the `ClientActions` struct to send all user requests.
  int SendRequests() {
    // Create and connect a FlightClient
    auto client_conn = SkytetherClient::ForLocation(service_loc);
    if (client_conn == nullptr) { return ERRCODE_CONN_CLIENT; }

    // Handle each action depending on what actions were set
    if (query_plan != nullptr) {
      // auto status_query = ExecuteQuery(*client_conn);
      auto status_query = ExecuteManyQueries(*client_conn);
      if (not status_query.ok()) {
        skytether::PrintError("Unable to execute query plan", status_query);
        return ERRCODE_API_QUERY;
      }
    }

    if (should_shutdown) {
      auto result_shutdown = client_conn->SendSignalShutdown();
      if (not result_shutdown.ok()) {
        skytether::PrintError("Unable to shutdown service", result_shutdown.status());
        return ERRCODE_API_SHUTDOWN;
      }
    }

    if (should_split and should_notsplit) {
      std::cerr << "Both [s] and [S] specified; can only specify one:" << std::endl
                << "\ts - Turn on cooperative decomposition"           << std::endl
                << "\tS - Turn off cooperative decomposition"          << std::endl
      ;
      return ERRCODE_API;
    }

    if (should_dereg) {
      auto result_dereg = client_conn->SendDeactivation(target_loc);
      if (not result_dereg.ok()) {
        skytether::PrintError("Unable to deregister service", result_dereg.status());
        return ERRCODE_API_DEREGISTER;
      }

      SkytetherDebugMsg("Service deregistered; shutting down...");
      auto service_conn = SkytetherClient::ForLocation(target_loc);
      if (service_conn == nullptr) { return ERRCODE_CONN_CLIENT; }

      auto result_shutdown = service_conn->SendSignalShutdown();
      if (not result_shutdown.ok()) {
        skytether::PrintError("Unable to shutdown service", result_shutdown.status());
        return ERRCODE_API_SHUTDOWN;
      }

      SkytetherDebugMsg("Service [" << target_loc.ToString() << "] shut down");
    }

    else if (should_notsplit) {
      auto result_decompoff = client_conn->SendDecompositionOff(target_loc);
      if (not result_decompoff.ok()) {
        skytether::PrintError("Unable to disable decomposition", result_decompoff.status());
        return ERRCODE_API_DEREGISTER;
      }
    }

    else if (should_split) {
      auto result_decompon = client_conn->SendDecompositionOn(target_loc);
      if (not result_decompon.ok()) {
        skytether::PrintError("Unable to enable decomposition", result_decompon.status());
        return ERRCODE_API_DEREGISTER;
      }
    }

    return 0;
  }
};


// ------------------------------
// Functions

int PrintHelp() {
    std::cout << "skytether-client"
              << " -l service-location-uri"
              << " -q path-to-plan-file"
              << " [-d location-to-deregister]"
              << " [-k]"
              << " [-h]"
              << std::endl
              << "-k sends shutdown request to service location"
              << std::endl
              << "-d sends request to de-register target location from metadata service"
              << std::endl
              << std::endl
    ;

    return 1;
}


// ------------------------------
// Main Logic

int main(int argc, char **argv) {
  ClientActions client_actions;

  // Parse each argument and internalize the provided option
  constexpr char  is_done_parsing = -1;
  const     char* opt_template    = "l:q:d:s:S:kh";
  char            parsed_opt;
  int             errcode_cli;

  while ((parsed_opt = (char) getopt(argc, argv, opt_template)) != is_done_parsing) {
    switch (parsed_opt) {

      case 'h': { return PrintHelp(); }

      case 'l': {
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.service_loc));
        SkytetherCheckErrCode(errcode_cli, "Failed to parse service location");
        break;
      }

      case 'q': {
        client_actions.query_plan = PlanMessage::FromFile(optarg);
        if (client_actions.query_plan == nullptr) {
          std::cerr << "Failed to parse plan file" << std::endl;
          return ERRCODE_FILE_PARSE;
        }

        client_actions.query_plan->payload->set_plan_id("simple-experiment");

        break;
      }

      case 'k': {
        client_actions.should_shutdown = true;
        break;
      }

      case 'd': {
        client_actions.should_dereg = true;
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.target_loc));
        SkytetherCheckErrCode(errcode_cli, "Failed to parse target location");
        break;
      }

      case 's': {
        client_actions.should_split = true;
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.target_loc));
        SkytetherCheckErrCode(errcode_cli, "Failed to parse target location");
        break;
      }

      case 'S': {
        client_actions.should_notsplit = true;
        errcode_cli = ParseArgLocationUri(optarg, &(client_actions.target_loc));
        SkytetherCheckErrCode(errcode_cli, "Failed to parse target location");
        break;
      }

      default: { break; }
    }
  }

  return client_actions.SendRequests();
}
