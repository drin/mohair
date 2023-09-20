// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

// >> Standard libs
#include <functional>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>

#include <vector>
#include <map>

// >> Third-party libs
#include <mpi.h>

#include "faodel/faodel-common/Common.hh"
#include "faodel/faodel-services/MPISyncStart.hh"
#include "faodel/lunasa/common/Helpers.hh"
#include "faodel/kelpie/Kelpie.hh"

// >> Internal libs
#include "arrow.hpp"


// >> Type Aliases
//  |> Types from standard lib
using std::string;
using std::vector;
using std::map;

//  |> common Faodel types

//      data objects
using KelpKey = kelpie::Key;
using LunaDO  = lunasa::DataObject;
using ArrowDO = faodel::ArrowDataObject;

//      core types
using FaoBucket = faodel::bucket_t;
using FaoStatus = faodel::rc_t;

using kelpie::KELPIE_OK;



// ------------------------------
// Functions

namespace mohair::adapters {

  const string default_pool_name { "/myplace" };
  string DefaultFaodelConfig(const string &pool_name);

  void InitializeMPI(int argc, char **argv, int *provided, int *rank, int *size);
  void BootstrapServices(string &faodel_config);
  void PrintStringObj(const string print_msg, const string string_obj);

  NamedTableProvider ProviderForFadoMap(map<KelpKey, LunaDO> &fado_map);

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
