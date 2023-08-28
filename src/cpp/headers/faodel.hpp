// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

// >> Standard libs
#include <functional>
#include <iostream>
#include <sstream>

// >> Third-party libs
#include <mpi.h>

#include "faodel/faodel-common/Common.hh"
#include "faodel/faodel-services/MPISyncStart.hh"
#include "faodel/lunasa/common/Helpers.hh"
#include "faodel/kelpie/Kelpie.hh"

// >> Aliases
using std::string;


// ------------------------------
// Functions

namespace mohair::adapters {

  const string default_pool_name { "/myplace" };
  string DefaultFaodelConfig(const string &pool_name);

  void InitializeMPI(int argc, char **argv, int *provided, int *rank, int *size);
  void BootstrapServices(string &faodel_config);
  void PrintStringObj(const string print_msg, const string string_obj);

  struct Faodel {
    string config_str;
    string pool_name;

    bool initialized;
    int  provided;
    int  mpi_rank;
    int  mpi_size;

    Faodel(const string &kpool_name, const string &service_config);
    Faodel();

    void Bootstrap(int argc, char **argv);
    void BootstrapWithKelpie(int argc, char **argv);
    void Finish();
    void PrintMPIInfo();

    kelpie::Pool ConnectToPool();
    lunasa::DataObject AllocateString(const string &str_obj);
    void FencedRankFn(int target_rank, std::function<void()> target_fn);
  };

} // namespace mohair::adapters
