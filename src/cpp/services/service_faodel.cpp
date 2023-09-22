// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "service_faodel.hpp"


// ------------------------------
// Type Aliases


// ------------------------------
// Functions

namespace mohair::services {


  unique_ptr<FlightServerBase> FaodelServiceWithDefaultLocation() {
    // grab a default location
    auto result_loc = default_location();
    if (not result_loc.ok()) {
      PrintError("Unable to construct default flight location", result_loc.status());
      return nullptr;
    }

    FlightServerOptions options { *result_loc };
    unique_ptr<FlightServerBase> service;

    // initialize the service
    auto status_init = service->Init(options);
    if (not status_init.ok()) {
      PrintError("Unable to initialize Faodel flight service", status_init);
      return nullptr;
    }

    // tell it to shutdown when SIGTERM is received
    auto status_handler = service->SetShutdownOnSignals({SIGTERM});
    if (not status_handler.ok()) {
      PrintError("Unable to set handler for SIGTERM", status_handler);
      return nullptr;
    }

    // return the service, ready to start
    return std::move(service);
  }
} // namespace: mohair::services


// ------------------------------
// Classes and Methods

namespace mohair::services {

  //  >> FaodelService

  Status FaodelService::Init(const FlightServerOptions &options) {
    // Call base Init and return if an error occurred
    auto parent_status = FlightServerBase::Init(options);
    if (not parent_status.ok()) { return parent_status; }

    // Initialize a Faodel adapter for this service to interact with
    faodel_if.BootstrapWithKelpie(/*argc=*/0, /*argv=*/nullptr);
    faodel_if.PrintMPIInfo();

    // register a compute function with kelpie
    kelpie::RegisterComputeFunction(
      "ExecuteWithAcero", mohair::adapters::ExecuteSubstrait
    );
  }

  Status FaodelService::ListFlights( const ServerCallContext   &context
                                    ,const Criteria            *criteria
                                    ,unique_ptr<FlightListing> *listings) {

    // TODO: figure out how to query catalog metadata
    /*
    vector<FlightInfo> flights = ...;
    *listings = std::make_unique<SimpleFlightListing>(flights);
    */

    return Status::NotImplemented("TODO");
  }

  Status FaodelService::GetFlightInfo( const ServerCallContext &context
                                      ,const FlightDescriptor  &request
                                      ,unique_ptr<FlightInfo>  *info) {
    return Status::NotImplemented("TODO");
  }

  // >= 13.0.0
  /*
  Status FaodelService::PollFlightInfo( const ServerCallContext &context
                                       ,const FlightDescriptor   &request
                                       ,unique_ptr<PollInfo>     *info) {
    return Status::NotImplemented("TODO");
  }
  */

  Status FaodelService::GetSchema( const ServerCallContext  &context
                                  ,const FlightDescriptor   &request
                                  ,unique_ptr<SchemaResult> *schema) {
    return Status::NotImplemented("TODO");
  }

  Status FaodelService::DoGet( const ServerCallContext      &context
                              ,const Ticket                 &request
                              ,unique_ptr<FlightDataStream> *writer) {
    return Status::NotImplemented("TODO");
  }

  Status FaodelService::DoPut( const ServerCallContext          &context
                              ,unique_ptr<FlightMessageReader>   reader
                              ,unique_ptr<FlightMetadataWriter>  writer) {
    return Status::NotImplemented("TODO");
  }

  Status FaodelService::DoExchange( const ServerCallContext         &context
                                   ,unique_ptr<FlightMessageReader>  reader
                                   ,unique_ptr<FlightMessageWriter>  writer) {
    return Status::NotImplemented("TODO");
  }

  Status FaodelService::DoAction( const ServerCallContext  &context
                                 ,const Action             &action
                                 ,unique_ptr<ResultStream> *result) {
    if (action.type == "query") {
      return ActionQuery(context, action.body, result);
    }

    // Catch all that returns Status::NotImplemented()
    return ActionUnknown(context, action.type);
  }

  Status FaodelService::ListActions( const ServerCallContext  &context
                                    ,vector<ActionType>       *actions) {
    return Status::NotImplemented("TODO");
  }


  Status FaodelService::ActionQuery( const ServerCallContext  &context
                                    ,const shared_ptr<Buffer>  plan_msg
                                    ,unique_ptr<ResultStream> *result) {
    // TODO receive the plan then execute it with Faodel.
    // This should use kelpie::Pool::Compute(<key>, "ExecuteWithAcero", plan_msg, <out_arg>)
    return Status::NotImplemented("TODO: query service");
  }

  Status FaodelService::ActionUnknown( const ServerCallContext &context
                                      ,const string             action_type) {
    return Status::NotImplemented("Unknown action: [", action_type, "]");
  }

} // namespace: mohair::services
