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
#pragma once

//  >> Common definitions for this library
#include "../mohair.hpp"

// >> integration with mohair query processing
#include "../query/plans.hpp"

//  >> Third-party libs
//    |> Arrow flight
#include <arrow/flight/api.h>


// ------------------------------
// Type aliases

//  >> Flight type aliases
using arrow::flight::FlightClient;
using arrow::flight::FlightServerBase;

using arrow::flight::FlightCallOptions;
using arrow::flight::FlightServerOptions;

using arrow::flight::FlightDataStream;
using arrow::flight::FlightMessageReader;
using arrow::flight::FlightMessageWriter;
using arrow::flight::FlightMetadataWriter;
using arrow::flight::SchemaResult;
using arrow::flight::ResultStream;

// >= 13.0.0
// using arrow::flight::PollInfo;
using arrow::flight::FlightInfo;
using arrow::flight::FlightListing;
using arrow::flight::SimpleFlightListing;

using arrow::flight::ServerCallContext;
using arrow::flight::Criteria;
using arrow::flight::FlightDescriptor;
using arrow::flight::Ticket;
using arrow::flight::Location;
using arrow::flight::Action;
using arrow::flight::ActionType;


// ------------------------------
// Classes

namespace mohair::services {

  struct MohairService : public FlightServerBase {

    //  >> FlightServerBase functions to override
    Status Init(const FlightServerOptions &options);

    virtual Status ListFlights(
       const ServerCallContext&   context
      ,const Criteria*            criteria
      ,unique_ptr<FlightListing>* listings
    );

    virtual Status GetFlightInfo(
       const ServerCallContext& context
      ,const FlightDescriptor&  request
      ,unique_ptr<FlightInfo>*  info
    );

    // >= 13.0.0
    /*
    Status PollFlightInfo(
       const ServerCallContext& context
      ,const FlightDescriptor&  request
      ,unique_ptr<PollInfo>*    info
    );
    */
 
    virtual Status GetSchema(
       const ServerCallContext  &context
      ,const FlightDescriptor   &request
      ,unique_ptr<SchemaResult> *schema
    );

    virtual Status DoGet(
       const ServerCallContext&      context
      ,const Ticket&                 request
      ,unique_ptr<FlightDataStream>* stream
    );

    virtual Status DoPut(
       const ServerCallContext&         context
      ,unique_ptr<FlightMessageReader>  reader
      ,unique_ptr<FlightMetadataWriter> writer
    );

    virtual Status DoExchange(
       const ServerCallContext         &context
      ,unique_ptr<FlightMessageReader>  reader
      ,unique_ptr<FlightMessageWriter>  writer
    );

    virtual Status DoAction(
       const ServerCallContext&  context
      ,const Action&             action
      ,unique_ptr<ResultStream>* result
    );

    virtual Status ListActions(
       const ServerCallContext& context
      ,vector<ActionType>*      actions
    );

    virtual Status ActionQuery(
       const ServerCallContext&  context
      ,const shared_ptr<Buffer>  plan_msg
      ,unique_ptr<ResultStream>* result
    );

    virtual Status ActionUnknown(
       const ServerCallContext& context
      ,const string action_type
    );

    //  >> Convenience functions
    virtual FlightInfo MakeFlightInfo();

  };

} // namespace: mohair::services


// ------------------------------
// Functions

namespace mohair::services {

  // >> Convenience functions
  Status SetDefaultLocation(Location *srv_loc);

  Status StartService(unique_ptr<FlightServerBase>& service) {
    // Initialize a location
    Location *srv_loc { nullptr };
    ARROW_RETURN_NOT_OK(SetDefaultLocation(srv_loc));

    // Create the service instance
    FlightServerOptions options { *srv_loc };

    std::cout << "Initializing service..." << std::endl;
    ARROW_RETURN_NOT_OK(service->Init(options));

    std::cout << "Setting shutdown signal handler..." << std::endl;
    ARROW_RETURN_NOT_OK(service->SetShutdownOnSignals({SIGTERM}));

    std::cout << "Starting service [localhost:" << service->port() << "]" << std::endl;
    ARROW_RETURN_NOT_OK(service->Serve());

    return Status::OK();
  }

} // namespace: mohair::services
