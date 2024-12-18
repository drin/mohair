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

// >> Common internal deps
#include "skytether.hpp"

// >> Third-party deps
#include <arrow/flight/api.h>


// ------------------------------
// Type aliases for convenience

namespace skytether::services {

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

  using arrow::flight::ResultStream;
  using arrow::flight::SchemaResult;
  using arrow::flight::FlightDataStream;
  using arrow::flight::FlightStreamReader;
  using arrow::flight::FlightStreamChunk;

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
  using arrow::flight::RecordBatchStream;

  using FlightResult = arrow::flight::Result;

} // namespace: skytether

