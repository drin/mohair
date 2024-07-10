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

// >> integration with duckdb
#if USE_DUCKDB
  // This brings in all useful duckdb dependencies
  #include "../engines/adapter_duckdb.hpp"
#endif


// ------------------------------
// Classes

#if USE_DUCKDB
  namespace mohair::services {

    struct DuckDBService : public MohairService {
      // >> Attributes
      unique_ptr<mohair::adapters::EngineDuckDB> duck_if;

      // >> Constructors
      DuckDBService();
      DuckDBService(fs::path db_fpath);

      // >> Convenience functions
      // NOTE: use default `MakeFlightInfo` for now
      // Result<FlightInfo> MakeFlightInfo() override;

      // >> Custom Flight API
      Status
      ActionQuery( const ServerCallContext&  context
                  ,const shared_ptr<Buffer>  plan_msg
                  ,unique_ptr<ResultStream>* result) override;

      // >> Standard Flight API
      Status
      MohairService::DoGet( const ServerCallContext&      context
                           ,const Ticket&                 request
                           ,unique_ptr<FlightDataStream>* stream) override;

    };

  } // namespace: mohair::services

#endif // essentially an include guard that uses USE_DUCKDB