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

//  >> Internal libs
#include "../mohair/mohair.hpp"

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
