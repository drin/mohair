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

// >> Internal deps and flight deps
#include "services/types.hpp"
#include "services/service_skytether.hpp"

// >> Engine deps
#if SKYTETHER_USE_DUCKDB
  #include "skytether/engines.hpp"
  #include "skytether/engines/duckdb/apidep_duckdb.hpp"
  #include "skytether/engines/duckdb/adapter_duckdb.hpp"
#endif


// ------------------------------
// Type aliases

using skytether::engines::EngineDuckDB;
using skytether::engines::DuckContext;
using skytether::engines::QueryStatus;


// ------------------------------
// Classes

#if SKYTETHER_USE_DUCKDB
  namespace skytether::services {

    struct DuckDBService : public EngineService {
      // >> Attributes
      unique_ptr<EngineDuckDB> engine;

      // >> Constructors and Deconstructors
      DuckDBService( unique_ptr<ServiceConfig>&& cfg
                    ,ShutdownCallback*           cb_custom
                    ,fs::path                    db_fpath);

      DuckDBService(unique_ptr<ServiceConfig>&& cfg, ShutdownCallback* cb_custom);
      DuckDBService(unique_ptr<ServiceConfig>&& cfg, fs::path db_fpath);
      DuckDBService(unique_ptr<ServiceConfig>&& cfg);

      virtual ~DuckDBService() = default;

      // >> Custom Flight API
      Status DoPlanPushdown  ( const ServerCallContext&  context
                              ,const shared_ptr<Buffer>  plan_msg
                              ,unique_ptr<ResultStream>* result) override;

      Status DoPlanExecution ( const ServerCallContext&  context
                              ,const shared_ptr<Buffer>  plan_msg
                              ,unique_ptr<ResultStream>* result) override;


      // >> Standard Flight API
      Status DoGet( const ServerCallContext&      context
                   ,const Ticket&                 request
                   ,unique_ptr<FlightDataStream>* stream) override;

      // >> Support methods
      Result<std::tuple<unique_ptr<Plan>, unique_ptr<SystemPlan>, size_t, string>>
      CoopDecomposePlan(unique_ptr<SystemPlan> sys_plan, const string& loc);

      Result<unique_ptr<SystemPlan>> EagerDecomposeDelegate(unique_ptr<SystemPlan> sys_plan);
      Result<unique_ptr<SystemPlan>> DelegatePushdown(unique_ptr<SystemPlan> sys_plan);

      Result<std::tuple<unique_ptr<Plan>, size_t, string>>
      EagerDecomposeExecute(SystemPlan& exec_sysplan, const string& loc);

      Result<std::tuple<unique_ptr<Plan>, unique_ptr<SystemPlan>, size_t, string>>
      LazyDecomposeExecute(unique_ptr<SystemPlan> pushback_sysplan, const string& loc);
    };

  } // namespace: skytether::services

#endif // essentially an include guard that uses SKYTETHER_USE_DUCKDB
