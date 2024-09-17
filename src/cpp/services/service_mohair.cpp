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

#include "service_mohair.hpp"


// ------------------------------
// Functions

// >> Internal functions
namespace mohair::services {

  void SerializeConfig( stringstream&        print_stream
                       ,string&              prefix
                       ,const ServiceConfig* service_cfg) {
    // Print root location
    print_stream << prefix << service_cfg->service_location() << std::endl;

    string downstream_prefix { prefix + "\t" };
    const auto& downstream_cfgs = service_cfg->downstream_services();
    for (size_t cfg_ndx = 0; cfg_ndx < downstream_cfgs.size(); ++cfg_ndx) {
      const auto& downstream_cfg = downstream_cfgs[cfg_ndx];

      if (not downstream_cfg.is_active()) { continue; }

      SerializeConfig(print_stream, downstream_prefix, &downstream_cfg);
    }
  }

} // namespace: mohair::services


// >> Convenience functions
namespace mohair::services {

  const string EngineService::hkey_queryticket { "QueryTicket" };

  // >> Public

  // Print service configurations using pre-order traversal
  void PrintConfig(ServiceConfig* service_cfg) {
    stringstream print_stream;
    string       empty_prefix;

    // Print root location
    print_stream << service_cfg->service_location() << std::endl;

    // Then recurse on each downstream config
    string downstream_prefix { empty_prefix + "\t" };
    const auto& downstream_cfgs = service_cfg->downstream_services();
    for (size_t cfg_ndx = 0; cfg_ndx < downstream_cfgs.size(); ++cfg_ndx) {
      const auto& downstream_cfg = downstream_cfgs[cfg_ndx];

      if (not downstream_cfg.is_active()) { continue; }

      SerializeConfig(print_stream, downstream_prefix, &downstream_cfg);
    }

    std::cout << print_stream.str() << std::endl;
  }

  // Create and assign a default location corresponding to any port on localhost
  int SetDefaultLocation(Location* srv_loc) {
    auto result_tcploc = Location::ForGrpcTcp("0.0.0.0", 0);
    if (not result_tcploc.ok()) {
      mohair::PrintError("Error creating default location", result_tcploc.status());
      return ERRCODE_CREATE_LOC;
    }

    *srv_loc = std::move(result_tcploc).ValueOrDie();
    return 0;
  }

  // Functions to start a service
  Status StartService(ServerAdapter& mohair_service, const Location& bind_loc) {
    MohairDebugMsg("Initializing options...");
    FlightServerOptions server_opts { bind_loc };

    MohairDebugMsg("Initializing service...");
    ARROW_RETURN_NOT_OK(mohair_service.Init(server_opts));

    MohairDebugMsg("Setting SIGTERM handler...");
    ARROW_RETURN_NOT_OK(mohair_service.SetShutdownOnSignals({SIGTERM}));

    MohairDebugMsg("Starting service [" << mohair_service.location().ToString() << "]");
    ARROW_RETURN_NOT_OK(mohair_service.Serve());

    return Status::OK();
  }

  Status StartService(ServerAdapter& mohair_service, const ServiceConfig& service_cfg) {
    auto result_bindloc = Location::Parse(service_cfg.service_location());
    if (not result_bindloc.ok()) {
      return Status::Invalid("Error parsing location from config");
    }

    MohairDebugMsg("Initializing options...");
    auto bind_loc = std::move(result_bindloc).ValueOrDie();
    FlightServerOptions server_opts { bind_loc };

    MohairDebugMsg("Initializing service...");
    ARROW_RETURN_NOT_OK(mohair_service.Init(server_opts));

    MohairDebugMsg("Setting SIGTERM handler...");
    ARROW_RETURN_NOT_OK(mohair_service.SetShutdownOnSignals({SIGTERM}));

    MohairDebugMsg("Starting service [" << mohair_service.location().ToString() << "]");
    ARROW_RETURN_NOT_OK(mohair_service.Serve());

    return Status::OK();
  }

} // namespace: mohair::services


// >> EngineService implementations
namespace mohair::services {

  Result<FlightInfo>
  EngineService::MakeFlightInfo(string partition_key, shared_ptr<Table> data_table) {
    // these are all dummy values for now
    vector<FlightEndpoint> cs_services;
    int64_t                count_rows  { 0    };
    int64_t                count_bytes { 0    };

    return FlightInfo::Make(
         *(data_table->schema())
        ,FlightDescriptor::Path({ partition_key })
        ,cs_services
        ,count_rows
        ,count_bytes
        ,/*is_ordered=*/true
        ,/*app_metadata=*/""
    );
  }

  Result<FlightInfo>
  EngineService::MakeFlightInfo(fs::path arrow_fpath, bool is_feather) {
    shared_ptr<Table> data_table;

    if (is_feather) {
      ARROW_ASSIGN_OR_RAISE(data_table, mohair::ReadIPCFile(arrow_fpath));
    }

    else {
      ARROW_ASSIGN_OR_RAISE(data_table, mohair::ReadIPCStream(arrow_fpath));
    }

    return MakeFlightInfo(arrow_fpath.filename().string(), data_table);
  }

  //  |> Default implementations for custom flight API
  Status
  EngineService::DoPlanPushdown( [[maybe_unused]] const ServerCallContext&  context
                                ,[[maybe_unused]] const shared_ptr<Buffer>  plan_msg
                                ,[[maybe_unused]] unique_ptr<ResultStream>* result) {
    // TODO: distinguish between pushdown and execution paths
    return Status::NotImplemented("TODO");
  }

  Status
  EngineService::DoPlanExecution( [[maybe_unused]] const ServerCallContext&  context
                                 ,[[maybe_unused]] const shared_ptr<Buffer>  plan_msg
                                 ,[[maybe_unused]] unique_ptr<ResultStream>* result) {
    // TODO: distinguish between pushdown and execution paths
    return Status::NotImplemented("TODO");
  }

  Status EngineService::DoViewChange( [[maybe_unused]] const ServerCallContext&  context
                                     ,                 const shared_ptr<Buffer>  config_msg
                                     ,[[maybe_unused]] unique_ptr<ResultStream>* result) {
    MohairDebugMsg("Receiving view change");
    ServiceConfig updated_cfg;

    string serialized_msg = config_msg->ToString();
    if (not updated_cfg.ParseFromString(serialized_msg)) {
      return Status::Invalid("Unable to parse service config for view change");
    }

    if (updated_cfg.service_location() == service_cfg.service_location()) {
      service_cfg = std::move(updated_cfg);

      MohairDebugMsg("New config:");
      PrintConfig(&service_cfg);

      return Status::OK();
    }

    stringstream err_msg;
    err_msg << "Cannot accept update for a different location."
            << std::endl
            << "\tExpected [" << service_cfg.service_location() << "]"
            << std::endl
            << "\tReceived [" << updated_cfg.service_location() << "]"
            << std::endl
    ;

    return Status::Invalid(err_msg.str());
  }


  //  |> Default implementations for standard flight API
  Status
  EngineService::DoServiceAction( const ServerCallContext&  context
                                 ,const Action&             action
                                 ,unique_ptr<ResultStream>* result) {

    // known actions (uses macros from apidep_flight.hpp)
    if (action.type == ActionQuery) {
      return DoPlanPushdown(context, action.body, result);
    }

    else if (action.type == ActionViewChange) {
      return DoViewChange(context, action.body, result);
    }

    else if (action.type == ActionShutdown) {
      return DoShutdown(context);
    }

    // Catch all that returns Status::NotImplemented()
    return DoUnknown(context, action.type);
  }

} // namespace: mohair::services
