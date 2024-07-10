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


// >> Convenience functions
namespace mohair::services {

  const string MohairService::hkey_queryticket { "QueryTicket" };

  Status SetDefaultLocation(Location* srv_loc) {
    // Assign into the location pointed to by `srv_loc`
    ARROW_ASSIGN_OR_RAISE(*srv_loc, Location::ForGrpcTcp("0.0.0.0", 0));

    return Status::OK();
  }


  Status StartService(unique_ptr<FlightServerBase>& service) {
    // Initialize a location
    MohairDebugMsg("Initializing location...");
    Location srv_loc;
    ARROW_RETURN_NOT_OK(SetDefaultLocation(&srv_loc));

    // Create the service instance
    MohairDebugMsg("Initializing options...");
    FlightServerOptions options { srv_loc };

    MohairDebugMsg("Initializing service...");
    ARROW_RETURN_NOT_OK(service->Init(options));

    MohairDebugMsg("Setting shutdown signal handler...");
    ARROW_RETURN_NOT_OK(service->SetShutdownOnSignals({SIGTERM}));

    MohairDebugMsg("Starting service [localhost:" << service->port() << "]");
    ARROW_RETURN_NOT_OK(service->Serve());

    return Status::OK();
  }


  Result<Location> MyLocation(MohairService& mohair_srv) {
  }

  Result<FlightEndpoint> MyEndpoint(MohairService& mohair_srv) {
  }

} // namespace: mohair::services


// >> Default MohairService implementations
namespace mohair::services {

  Result<FlightInfo>
  MohairService::MakeFlightInfo(string partition_key, shared_ptr<Table> data_table) {
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
  MohairService::MakeFlightInfo(fs::path arrow_fpath, bool is_feather) {
    shared_ptr<Table> data_table;

    if (is_feather) {
      ARROW_ASSIGN_OR_RAISE(data_table, mohair::ReadIPCFile(arrow_fpath));
    }

    else {
      ARROW_ASSIGN_OR_RAISE(data_table, mohair::ReadIPCStream(arrow_fpath));
    }

    return MakeFlightInfo(arrow_fpath.filename(), data_table);
  }

  //  |> Default implementations for custom flight API
  Status MohairService::ActionQuery( const ServerCallContext&  context
                                    ,const shared_ptr<Buffer>  plan_msg
                                    ,unique_ptr<ResultStream>* result) {
    return Status::NotImplemented("TODO");
  }

  Status MohairService::ActionUnknown( const ServerCallContext& context
                                      ,const string             action_type) {
    return Status::NotImplemented("Unknown action: [", action_type, "]");
  }


  //  |> Default implementations for standard flight API
  Status
  MohairService::ListFlights( [[maybe_unused]] const ServerCallContext&   context
                             ,[[maybe_unused]] const Criteria*            criteria
                             ,[[maybe_unused]] unique_ptr<FlightListing>* listings) {
    return Status::NotImplemented("TODO");
  }

  Status
  MohairService::GetFlightInfo( [[maybe_unused]] const ServerCallContext& context
                               ,[[maybe_unused]] const FlightDescriptor&  request
                               ,[[maybe_unused]] unique_ptr<FlightInfo>*  info) {
    return Status::NotImplemented("TODO");
  }

  Status
  MohairService::PollFlightInfo( [[maybe_unused]] const ServerCallContext& context
                                ,[[maybe_unused]] const FlightDescriptor&  request
                                ,[[maybe_unused]] unique_ptr<PollInfo>*    info) {
    return Status::NotImplemented("TODO");
  }

  Status
  MohairService::GetSchema( [[maybe_unused]] const ServerCallContext  &context
                           ,[[maybe_unused]] const FlightDescriptor   &request
                           ,[[maybe_unused]] unique_ptr<SchemaResult> *schema) {
    return Status::NotImplemented("TODO");
  }

  Status
  MohairService::DoGet( [[maybe_unused]] const ServerCallContext&      context
                       ,[[maybe_unused]] const Ticket&                 request
                       ,[[maybe_unused]] unique_ptr<FlightDataStream>* stream) {
    return Status::NotImplemented("TODO");
  }

  Status
  MohairService::DoPut( [[maybe_unused]] const ServerCallContext&         context
                       ,[[maybe_unused]] unique_ptr<FlightMessageReader>  reader
                       ,[[maybe_unused]] unique_ptr<FlightMetadataWriter> writer) {
    return Status::NotImplemented("TODO");
  }

  Status
  MohairService::DoExchange( [[maybe_unused]] const ServerCallContext         &context
                            ,[[maybe_unused]] unique_ptr<FlightMessageReader>  reader
                            ,[[maybe_unused]] unique_ptr<FlightMessageWriter>  writer) {
    return Status::NotImplemented("TODO");
  }

  Status
  MohairService::DoAction( const ServerCallContext&  context
                          ,const Action&             action
                          ,unique_ptr<ResultStream>* result) {

    // known actions
    if (action.type == "query") { return ActionQuery(context, action.body, result); }

    // Catch all that returns Status::NotImplemented()
    return ActionUnknown(context, action.type);
  }

  Status
  MohairService::ListActions( [[maybe_unused]] const ServerCallContext& context
                             ,[[maybe_unused]] vector<ActionType>*      actions) {
    return Status::NotImplemented("TODO");
  }

} // namespace: mohair::services
