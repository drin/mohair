// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "../headers/adapter_faodel.hpp"


// Type Aliases
using std::string;
using std::stringstream;

using std::endl;

// ------------------------------
// Classes and Functions

namespace mohair::services {

  // ------------------------------
  // Convenience Methods for Arrow Flight
  Result<Location> default_location() {
    Location default_loc;
    status_setloc = arrow::flight::Location::ForGrpcTcp("0.0.0.0", 0, &default_loc);
    if (status_setloc.ok()) { return std::move(default_loc); }

    return status_setloc;
  }

  unique_ptr<FlightServerBase> FaodelServiceWithDefaultLocation() {
    // grab a default location
    result_loc = default_location();
    if (not result_loc.ok()) {
      PrintError("Unable to construct default flight location", result_loc.status());
      return nullptr;
    }

    FlightServerOptions options { *result_loc };
    unique_ptr<FlightServerBase> service;

    // initialize the service
    status_init = service->Init(options);
    if (not status_init.ok()) {
      PrintError("Unable to initialize Faodel flight service", status_init);
      return nullptr;
    }

    // tell it to shutdown when SIGTERM is received
    status_handler = service->SetShutdownOnSignals({SIGTERM});
    if (not status_handler.ok()) {
      PrintError("Unable to set handler for SIGTERM", status_handler);
      return nullptr;
    }

    // return the service, ready to start
    return std::move(service);
  }

  // ------------------------------
  // Methods for FaodelService

  Status FaodelService::Init(const FlightServerOptions &options) h
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
    vector<FlightInfo> flights = ...;
    *listings = std::make_unique<SimpleFlightListing>(flights);

    return Status::NotImplemented("TODO");
  }

  Status GetFlightInfo( const ServerCallContext &context
                       ,const FlightDescriptor  &request
                       ,unique_ptr<FlightInfo>  *info) {
    return Status::NotImplemented("TODO");
  }

  Status GetSchema( const ServerCallContext  &context
                   ,const FlightDescriptor   &request
                   ,unique_ptr<SchemaResult> *schema) {
    return Status::NotImplemented("TODO");
  }

  Status DoExchange( const ServerCallContext         &context
                    ,unique_ptr<FlightMessageReader>  reader
                    ,unique_ptr<FlightMessageWriter>  writer) {
    return Status::NotImplemented("TODO");
  }

  Status DoPut( const ServerCallContext          &context
               ,unique_ptr<FlightMessageReader>   reader
               ,unique_ptr<FlightMetadataWriter>  writer) {
    return Status::NotImplemented("TODO");
  }

  Status DoGet( const ServerCallContext         &context
               ,const Ticket                    &request
               ,unique_ptr<FlightMessageWriter>  writer) {
    return Status::NotImplemented("TODO");
  }

  Status DoAction( const ServerCallContext  &context
                  ,const Action             &action
                  ,unique_ptr<ResultStream> *result) {
    if (action.type == "query") {
      return ActionQuery(context, action.body, result);
    }

    // Catch all that returns Status::NotImplemented()
    return ActionUnknown(context, action.type);
  }

  Status ActionQuery( const ServerCallContext  &context
                     ,shared_ptr<Buffer>       &plan_msg
                     ,unique_ptr<ResultStream> *result) {
    // TODO receive the plan then execute it with Faodel.
    // This should use kelpie::Pool::Compute(<key>, "ExecuteWithAcero", plan_msg, <out_arg>)
    return Status::NotImplemented("TODO: query service");
  }

  Status ActionUnknown(const ServerCallContext &context, string &action_type) {
    return Status::NotImplemented("Unknown action: [", action_type, "]");
  }

} // namespace: mohair::services
