// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies
#pragma once

// >> Internal libs

//    |> flight deps
#include "flight_internal.hpp"

//    |> integration with faodel and faodel-arrow
#include "../engines/adapter_faodel.hpp"

//    |> integration with mohair query processing
#include "../mohair/plans.hpp"


// ------------------------------
// Functions

namespace mohair::services {

  // >> Convenience functions
  Result<Location> default_location();
  Status StartDefaultFaodelService();

} // namespace: mohair::services


// ------------------------------
// Classes

namespace mohair::services {

  // >> Classes
  struct FaodelService : public FlightServerBase {
    mohair::adapters::Faodel faodel_if;
    kelpie::Pool             faodel_pool;

    //  >> FlightServerBase functions to override

    Status           Init(const FlightServerOptions &options       );

    Status    ListFlights( const ServerCallContext   &context
                          ,const Criteria            *criteria
                          ,unique_ptr<FlightListing> *listings     ) override;

    Status  GetFlightInfo( const ServerCallContext &context
                          ,const FlightDescriptor  &request
                          ,unique_ptr<FlightInfo>  *info           ) override;

    // >= 13.0.0
    /*
    Status PollFlightInfo( const ServerCallContext &context
                          ,const FlightDescriptor   &request
                          ,unique_ptr<PollInfo>     *info          ) override;
    */
 
    Status      GetSchema( const ServerCallContext  &context
                          ,const FlightDescriptor   &request
                          ,unique_ptr<SchemaResult> *schema        ) override;

    Status          DoGet( const ServerCallContext         &context
                          ,const Ticket                    &request
                          ,unique_ptr<FlightDataStream>    *stream ) override;

    Status          DoPut( const ServerCallContext          &context
                          ,unique_ptr<FlightMessageReader>   reader
                          ,unique_ptr<FlightMetadataWriter>  writer) override;

    Status     DoExchange( const ServerCallContext         &context
                          ,unique_ptr<FlightMessageReader>  reader
                          ,unique_ptr<FlightMessageWriter>  writer ) override;

    Status       DoAction( const ServerCallContext  &context
                          ,const Action             &action
                          ,unique_ptr<ResultStream> *result        ) override;

    Status    ListActions( const ServerCallContext  &context
                          ,vector<ActionType>       *actions       ) override;

    Status    ActionQuery( const ServerCallContext  &context
                          ,const shared_ptr<Buffer>  plan_msg
                          ,unique_ptr<ResultStream> *result        );

    Status  ActionUnknown( const ServerCallContext &context
                          ,const string action_type                );

    //  >> Convenience functions
    FlightInfo MakeFlightInfo();

  };

} // namespace: mohair::services
