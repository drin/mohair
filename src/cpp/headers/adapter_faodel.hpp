// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies
#pragma once

// >> Internal libs
#include "mohair.hpp"

// >> Standard libs
#include <map>

// >> Third-party libs
#include <mpi.h>

//  |> Core faodel and MPI interface
#include <faodel/faodel-services/MPISyncStart.hh>
#include <faodel/faodel-common/Common.hh>
#include <faodel/faodel-common/FaodelTypes.hh>

//  |> Faodel "plugins" (kelpie, faodel-arrow, etc.)
#include <faodel/lunasa/common/Helpers.hh>

#include <faodel/kelpie/Kelpie.hh>
#include <faodel-arrow/ArrowDataObject.hh>


// >> Type Aliases
//  |> Types from standard lib
using std::map;

//  |> common Faodel types

//      data objects
using KelpKey = kelpie::Key;
using LunaDO  = lunasa::DataObject;
using ArrowDO = faodel::ArrowDataObject;

//      core types
using FaoBucket = faodel::bucket_t;
using FaoStatus = faodel::rc_t;


// ------------------------------
// Functions

namespace mohair::adapters {

  const string default_pool_name { "/myplace" };
  string DefaultFaodelConfig(const string &pool_name);

  void InitializeMPI(int argc, char **argv, int *provided, int *rank, int *size);
  void BootstrapServices(string &faodel_config);
  void PrintStringObj(const string print_msg, const string string_obj);

  // Functions to support interfacing with Acero and other execution engines
  NamedTableProvider ProviderForFadoMap(map<KelpKey, LunaDO> &fado_map);
  FaoStatus ExecuteSubstrait(        FaoBucket       b
                              ,const KelpKey         k
                              ,const string         &args
                              ,map<KelpKey, LunaDO>  fado_map
                              ,LunaDO               *ext_ldo);

  struct Faodel {
    // state for managing faodel
    string               config_str;
    string               pool_name;
    map<KelpKey, LunaDO> fado_map;

    // state for managing MPI
    bool initialized;
    int  provided;
    int  mpi_rank;
    int  mpi_size;

    Faodel(const string &kpool_name, const string &service_config);
    Faodel();

    // Functions for interfacing with Faodel libraries
    kelpie::Pool       ConnectToPool();
    lunasa::DataObject AllocateString(const string &str_obj);

    // Functions for MPI integration
    void Bootstrap(int argc, char **argv);
    void BootstrapWithKelpie(int argc, char **argv);
    void PrintMPIInfo();
    void Finish();
    void FencedRankFn(int target_rank, std::function<void()> target_fn);

    // Functions for Arrow and Acero integration
    NamedTableProvider FadoTableProvider();
  };

} // namespace mohair::adapters


namespace mohair::services {

  // >> Convenience functions
  Result<Location> default_location();

  // >> Classes
  struct FaodelService : public FlightServerBase {
    mohair::adapters::Faodel faodel_if;

    Status Init(const FlightServerOptions &options);

    FlightInfo MakeFlightInfo();

    Status ListFlights( const ServerCallContext   &context
                       ,const Criteria            *criteria
                       ,unique_ptr<FlightListing> *listings) override;

    Status GetFlightInfo( const ServerCallContext &context
                         ,const FlightDescriptor  &request
                         ,unique_ptr<FlightInfo>  *info);

    Status GetSchema( const ServerCallContext  &context
                     ,const FlightDescriptor   &request
                     ,unique_ptr<SchemaResult> *schema);

    Status DoExchange( const ServerCallContext         &context
                      ,unique_ptr<FlightMessageReader>  reader
                      ,unique_ptr<FlightMessageWriter>  writer);

    Status DoPut( const ServerCallContext          &context
                 ,unique_ptr<FlightMessageReader>   reader
                 ,unique_ptr<FlightMetadataWriter>  writer);

    Status DoGet( const ServerCallContext         &context
                 ,const Ticket                    &request
                 ,unique_ptr<FlightMessageWriter>  writer);

    Status DoAction( const ServerCallContext  &context
                    ,const Action             &action
                    ,unique_ptr<ResultStream> *result);

    Status ActionQuery( const ServerCallContext  &context
                       ,shared_ptr<Buffer>       &plan_msg
                       ,unique_ptr<ResultStream> *result);

    Status ActionUnknown(const ServerCallContext &context, string &action_type);

  };

} // namespace: mohair::services
