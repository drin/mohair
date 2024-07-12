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

// >> Third-party libs
#include <arrow/flight/api.h>

// >> Internal libs
#include "../query/topology.pb.h"


// ------------------------------
// Type aliases

//  >> Flight type aliases
using arrow::flight::FlightClient;
using arrow::flight::FlightCallOptions;

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

using mohair::ServiceConfig;
using mohair::DeviceClass;


// ------------------------------
// Classes

namespace mohair::services {

  struct MohairClient {
    // >> Attributes
    unique_ptr<FlightClient> conn;
    FlightCallOptions        conn_opts;

    // >> Constructors
    MohairClient(unique_ptr<FlightClient>&& client): conn(std::move(client)) {}

    // >> Methods
    Result<unique_ptr<ResultStream>> RegisterService(Location& service_loc);

    // >> Static builders
    static Result<unique_ptr<MohairClient>>
    ForTcpLocation(const string& host, const int port);
  };

} // namespace: mohair::services

