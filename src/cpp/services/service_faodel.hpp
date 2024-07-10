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

// >> flight deps
#include "service_mohair.hpp"

// >> integration with faodel and faodel-arrow
#if USE_FAODEL
  #include "../engines/adapter_faodel.hpp"
#endif


// ------------------------------
// Classes

#if USE_FAODEL
  namespace mohair::services {

    struct FaodelService : public virtual MohairService {
      mohair::adapters::Faodel faodel_if;
      kelpie::Pool             faodel_pool;

      FaodelService();

      Status ListFlights(
         const ServerCallContext&   context
        ,const Criteria*            criteria
        ,unique_ptr<FlightListing>* listings
      ) override;

      Status GetFlightInfo(
         const ServerCallContext& context
        ,const FlightDescriptor&  request
        ,unique_ptr<FlightInfo>*  info
      ) override;
   
      Status GetSchema(
         const ServerCallContext&  context
        ,const FlightDescriptor&   request
        ,unique_ptr<SchemaResult>* schema
      ) override;

      Status DoGet(
         const ServerCallContext&      context
        ,const Ticket&                 request
        ,unique_ptr<FlightDataStream>* stream
      ) override;

      Status DoPut(
         const ServerCallContext&         context
        ,unique_ptr<FlightMessageReader>  reader
        ,unique_ptr<FlightMetadataWriter> writer
      ) override;

      Status DoExchange(
         const ServerCallContext&        context
        ,unique_ptr<FlightMessageReader> reader
        ,unique_ptr<FlightMessageWriter> writer
      ) override;

      Status DoAction(
         const ServerCallContext&  context
        ,const Action&             action
        ,unique_ptr<ResultStream>* result
      ) override;

      Status ListActions(
         const ServerCallContext& context
        ,vector<ActionType>*      actions
      ) override;

      Status ActionQuery(
         const ServerCallContext&  context
        ,const shared_ptr<Buffer>  plan_msg
        ,unique_ptr<ResultStream>* result
      ) override;

      Status ActionUnknown(
         const ServerCallContext& context
        ,const string             action_type
      ) override;

      //  >> Convenience functions
      FlightInfo MakeFlightInfo();

    };

  } // namespace: mohair::services

#endif // essentially an include guard that uses USE_FAODEL
