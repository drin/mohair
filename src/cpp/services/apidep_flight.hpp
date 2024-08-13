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
#pragma once

// >> Standard libs
#include <memory>
#include <string>
#include <iostream>

// >> Third-party libs
#include "../apidep_arrow.hpp"

#include <arrow/flight/api.h>


// ------------------------------
// Type aliases for convenience

// >> Types from standard lib
using std::unique_ptr;
using std::string;

// >> Server/client types
using arrow::flight::FlightClient;
using arrow::flight::FlightServerBase;

// >> Option types
using arrow::flight::FlightCallOptions;
using arrow::flight::FlightServerOptions;

// >> Message-passing types
using arrow::flight::FlightMessageReader;
using arrow::flight::FlightMessageWriter;
using arrow::flight::FlightMetadataWriter;

using arrow::flight::FlightDataStream;
using arrow::flight::SchemaResult;
using arrow::flight::ResultStream;

using arrow::flight::PollInfo;
using arrow::flight::FlightInfo;

using arrow::flight::FlightListing;
using arrow::flight::SimpleFlightListing;

using arrow::flight::Criteria;
using arrow::flight::ServerCallContext;
using arrow::flight::Action;
using arrow::flight::ActionType;
using arrow::flight::FlightDescriptor;
using arrow::flight::FlightEndpoint;
using arrow::flight::Ticket;
using arrow::flight::Location;

// >> Concrete types
using arrow::flight::SimpleResultStream;
using FlightResult = arrow::flight::Result;


// ------------------------------
// Macros

// >> Mohair-supported action names

// General actions
#define ActionShutdown   "service-shutdown"

// Topology-specific actions
#define ActionActivate   "topology-activate"
#define ActionDeactivate "topology-deactivate"

// Engine-specific actions
#define ActionViewChange "view-change"
#define ActionQuery      "mohair-query"



// ------------------------------
// Classes

// >> Support classes
namespace mohair::services {

  // Type forwards
  struct ClientAdapter;

  // Types for "handler" support

  //! Default handler implementation for server shutdown
  struct ShutdownCallback {
    virtual ~ShutdownCallback() = default;
    virtual Status operator()() { return Status::OK(); }
  };

} // namespace: mohair::services


// >> Adapter classes
namespace mohair::services {

  //! An adapter for some insulation from the FlightClient interface.
  struct ClientAdapter {
    unique_ptr<FlightClient> client;
    FlightCallOptions        rpc_opts;

    // >> Destructors and Constructors
    virtual ~ClientAdapter() = default;
    ClientAdapter(unique_ptr<FlightClient>&& client): client(std::move(client)) {}

    // >> Methods
    Result<unique_ptr<ResultStream>> SendSignalShutdown();
  };


  //! An adapter for some insulation from the FlightServerBase interface. 
  /**
   * This provides a layer of indirection between our internal implementation of custom
   * flight services and the interface provided by flight::FlightServerBase. The Standard
   * Flight API will be implemented by this class by delegating to a custom API. The
   * custom API will then be overridden by mohair services as appropriate.
   */
  struct ServerAdapter : public FlightServerBase {

    // >> Custom Attributes
    ShutdownCallback* cb_shutdown;

    // >> Destructors and Constructors
    virtual ~ServerAdapter() = default;

    ServerAdapter(ShutdownCallback* cb_custom)
      : FlightServerBase(), cb_shutdown(cb_custom) {}

    ServerAdapter(): ServerAdapter(nullptr) {}

    // >> Custom Flight API
    virtual Status DoServiceAction( const ServerCallContext&  context
                                   ,const Action&             action
                                   ,unique_ptr<ResultStream>* result);

    virtual Status DoShutdown(const ServerCallContext& context);
    virtual Status DoUnknown (const ServerCallContext& context, const string action_type);

    // >> Standard Flight API
    /** TODO: uncomment as each is implemented **/

    // control flow functions
    /*
    Status
    ListFlights   ( const ServerCallContext&   context
                   ,const Criteria*            criteria
                   ,unique_ptr<FlightListing>* listings) override;

    Status
    GetFlightInfo ( const ServerCallContext& context
                   ,const FlightDescriptor&  request
                   ,unique_ptr<FlightInfo>*  info) override;

    Status
    PollFlightInfo( const ServerCallContext& context
                   ,const FlightDescriptor&  request
                   ,unique_ptr<PollInfo>*    info) override;

    Status
    ListActions   ( const ServerCallContext& context
                   ,vector<ActionType>*      actions) override;
    */

    Status
    DoAction  ( const ServerCallContext&  context
               ,const Action&             action
               ,unique_ptr<ResultStream>* result) override;

    // data flow functions
    /*
    Status
    GetSchema ( const ServerCallContext  &context
               ,const FlightDescriptor   &request
               ,unique_ptr<SchemaResult> *schema) override;

    Status
    DoGet     ( const ServerCallContext&      context
               ,const Ticket&                 request
               ,unique_ptr<FlightDataStream>* stream) override;

    Status
    DoPut     ( const ServerCallContext&         context
               ,unique_ptr<FlightMessageReader>  reader
               ,unique_ptr<FlightMetadataWriter> writer) override;

    Status
    DoExchange( const ServerCallContext         &context
               ,unique_ptr<FlightMessageReader>  reader
               ,unique_ptr<FlightMessageWriter>  writer) override;
    */
  };

} // namespace: mohair::services
