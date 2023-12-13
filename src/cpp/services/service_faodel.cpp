// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

// >> Configuration-based macros
#include "../mohair-config.hpp"

#include "service_faodel.hpp"


// ------------------------------
// Functions

namespace mohair::services {

  Status StartDefaultFaodelService() {
    unique_ptr<FlightServerBase> faodel_service = std::make_unique<FaodelService>();
    return StartService(faodel_service);
  }

} // namespace: mohair::services


// ------------------------------
// Classes and Methods

namespace mohair::services {

  //  >> FaodelService

  Status FaodelService::Init(const FlightServerOptions &options) {
    // Call base Init and return if an error occurred
    std::cout << "Initializing Base Server" << std::endl;
    auto parent_status = FlightServerBase::Init(options);
    if (not parent_status.ok()) { return parent_status; }

    // Initialize a Faodel adapter for this service to interact with
    std::cout << "Bootstrapping Faodel" << std::endl;
    faodel_if.BootstrapWithKelpie(/*argc=*/0, /*argv=*/nullptr);
    faodel_if.PrintMPIInfo();

    // register a compute function with kelpie
    faodel_if.RegisterEngineAcero();
    faodel_pool = faodel_if.ConnectToPool();

    return Status::OK();
  }

  Status FaodelService::ListFlights( [[maybe_unused]] const ServerCallContext   &context
                                    ,[[maybe_unused]] const Criteria            *criteria
                                    ,[[maybe_unused]] unique_ptr<FlightListing> *listings) {

    // TODO: figure out how to query catalog metadata
    /*
    vector<FlightInfo> flights = ...;
    *listings = std::make_unique<SimpleFlightListing>(flights);
    */

    return Status::NotImplemented("TODO");
  }

  Status FaodelService::GetFlightInfo( [[maybe_unused]] const ServerCallContext &context
                                      ,[[maybe_unused]] const FlightDescriptor  &request
                                      ,[[maybe_unused]] unique_ptr<FlightInfo>  *info) {
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

  Status FaodelService::GetSchema( [[maybe_unused]] const ServerCallContext  &context
                                  ,[[maybe_unused]] const FlightDescriptor   &request
                                  ,[[maybe_unused]] unique_ptr<SchemaResult> *schema) {
    return Status::NotImplemented("TODO");
  }

  Status FaodelService::DoGet( [[maybe_unused]] const ServerCallContext      &context
                              ,[[maybe_unused]] const Ticket                 &request
                              ,[[maybe_unused]] unique_ptr<FlightDataStream> *writer) {
    return Status::NotImplemented("TODO");
  }

  Status FaodelService::DoPut( [[maybe_unused]] const ServerCallContext          &context
                              ,[[maybe_unused]] unique_ptr<FlightMessageReader>   reader
                              ,[[maybe_unused]] unique_ptr<FlightMetadataWriter>  writer) {
    return Status::NotImplemented("TODO");
  }

  Status FaodelService::DoExchange( [[maybe_unused]] const ServerCallContext         &context
                                   ,[[maybe_unused]] unique_ptr<FlightMessageReader>  reader
                                   ,[[maybe_unused]] unique_ptr<FlightMessageWriter>  writer) {
    return Status::NotImplemented("TODO");
  }

  Status FaodelService::DoAction( [[maybe_unused]] const ServerCallContext  &context
                                 ,                 const Action             &action
                                 ,                 unique_ptr<ResultStream> *result) {
    if (action.type == "query") {
      return ActionQuery(context, action.body, result);
    }

    // Catch all that returns Status::NotImplemented()
    return ActionUnknown(context, action.type);
  }

  Status FaodelService::ListActions( [[maybe_unused]] const ServerCallContext  &context
                                    ,[[maybe_unused]] vector<ActionType>       *actions) {
    return Status::NotImplemented("TODO");
  }


  Status FaodelService::ActionQuery( [[maybe_unused]] const ServerCallContext  &context
                                    ,                 const shared_ptr<Buffer>  plan_msg
                                    ,[[maybe_unused]] unique_ptr<ResultStream> *result) {
    // TODO receive the plan then execute it with Faodel.
    // faodel_if.ExecuteEngineAcero(faodel_pool, <key>, plan_msg);
    string plan_data = plan_msg->ToString();
    auto substrait_plan = mohair::SubstraitPlanFromString(plan_data);
    return Status::NotImplemented("TODO: query service");
  }

  Status FaodelService::ActionUnknown( [[maybe_unused]] const ServerCallContext &context
                                      ,                 const string             action_type) {
    return Status::NotImplemented("Unknown action: [", action_type, "]");
  }

} // namespace: mohair::services
