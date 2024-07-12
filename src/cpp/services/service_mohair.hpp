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

// >> Common definitions for this library
#include "../mohair.hpp"

// >> integration with mohair query processing
#include "../query/plans.hpp"

// >> Third-party libs
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

using arrow::flight::SimpleResultStream;
using FlightResult = arrow::flight::Result;


// ------------------------------
// Classes

namespace mohair::services {

  // ----------
  // Hash implementations for mohair services (implicit definition fails)

  // >> For Tickets
  struct HashFunctorMohairTicket {
    // A mohair ticket may be a service location or a query identifier
    static const std::hash<string> HashMohairTicketId;

    std::size_t operator()(const Ticket& mohair_ticket) const {
      return HashMohairTicketId(mohair_ticket.ticket);
    }
  };

  // >> For Flight Locations
  struct HashFunctorMohairLocation {
    // A location is essentially a URI
    static const std::hash<string> HashMohairLocation;

    std::size_t operator()(const Location& mohair_location) const {
      return HashMohairLocation(mohair_location.ToString());
    }
  };


  // ----------
  // Service implementations

  struct MohairService : public FlightServerBase {

    static const string hkey_queryticket;

    // >> Convenience functions
    virtual Result<FlightInfo>
    MakeFlightInfo(string partition_key, shared_ptr<Table> data_table);

    virtual Result<FlightInfo>
    MakeFlightInfo(fs::path arrow_fpath, bool is_feather);

    // >> Custom Flight API
    virtual Status ActionQuery(
       const ServerCallContext&  context
      ,const shared_ptr<Buffer>  plan_msg
      ,unique_ptr<ResultStream>* result
    );

    virtual Status ActionUnknown(
       const ServerCallContext& context
      ,const string action_type
    );

    // >> Standard Flight API

    Status ListFlights( const ServerCallContext&   context
                       ,const Criteria*            criteria
                       ,unique_ptr<FlightListing>* listings) override;

    Status GetFlightInfo( const ServerCallContext& context
                         ,const FlightDescriptor&  request
                         ,unique_ptr<FlightInfo>*  info) override;

    Status PollFlightInfo( const ServerCallContext& context
                          ,const FlightDescriptor&  request
                          ,unique_ptr<PollInfo>*    info) override;

    Status GetSchema( const ServerCallContext  &context
                     ,const FlightDescriptor   &request
                     ,unique_ptr<SchemaResult> *schema) override;

    Status DoGet( const ServerCallContext&      context
                 ,const Ticket&                 request
                 ,unique_ptr<FlightDataStream>* stream) override;

    Status DoPut( const ServerCallContext&         context
                 ,unique_ptr<FlightMessageReader>  reader
                 ,unique_ptr<FlightMetadataWriter> writer) override;

    Status DoExchange( const ServerCallContext         &context
                      ,unique_ptr<FlightMessageReader>  reader
                      ,unique_ptr<FlightMessageWriter>  writer) override;

    Status DoAction( const ServerCallContext&  context
                    ,const Action&             action
                    ,unique_ptr<ResultStream>* result) override;

    Status ListActions( const ServerCallContext& context
                       ,vector<ActionType>*      actions) override;

  };

} // namespace: mohair::services


// ------------------------------
// Functions
namespace mohair::services {

  // >> Convenience functions
  Status SetDefaultLocation(Location *srv_loc);
  Status StartService(unique_ptr<FlightServerBase>& service);

  Result<Location>       MyLocation(MohairService& mohair_srv);
  Result<FlightEndpoint> MyEndpoint(MohairService& mohair_srv);

  // >> Engine-specific functions
  Status StartMohairDuckDB();

} // namespace: mohair::services
