// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

// >> Standard libs
#include <iostream>
#include <sstream>

// >> Third-party libs
#include <mpi.h>

#include "faodel/faodel-common/Common.hh"
#include "faodel/faodel-services/MPISyncStart.hh"
#include "lunasa/common/Helpers.hh"
#include "kelpie/Kelpie.hh"


// ------------------------------
// Functions
namespace mohair::adapters {

  constexpr string default_pool_name { "/myplace" };
  string DefaultFaodelConfig(string &pool_name = default_pool_name);

  void InitializeMPI(int argc, char **argv, int *provided, int *rank, int *size);
  void BootstrapServices(std::string &faodel_config);
  void PrintStringObj(const std::string print_msg, const std::string string_obj);

  struct Faodel {
    string config_str;
    string pool_name;

    bool initialized;
    int  provided;
    int  mpi_rank;
    int  mpi_size;

    Faodel::Faodel(string &service_config);
    Faodel::Faodel();

    void Bootstrap(int argc, char **argv);
    void BootstrapWithKelpie(int argc, char **argv);
    void Finish();
    auto ConnectToPool();
    auto AllocateString(string &str_obj);
  };

} // mohair::faodel
