// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "faodel.hpp"


// type aliases
using std::string;
using std::stringstream;


// ------------------------------
// Module variables

// Default faodel configuration
namespace mohair::adapters {

  string DefaultFaodelConfig(string &pool_name) {
    stringstream config_ss;

    config_ss << "# Use mpisyncstart to create a DHT (across all our nodes)"
              << "# Name of dht is '/myplace'"
              << "mpisyncstart.enable    true"
              << "dirman.type            centralized"
              << "dirman.root_node_mpi   0"
              << "dirman.resources_mpi[] dht:" << pool_name << " ALL"

              << "# Uncomment these options to get debug info for each component"
              << "bootstrap.debug true"
              << "dirman.debug    true"
              << "kelpie.debug    true"
    ;

    return config_ss.string();
  }

  Faodel::Faodel(string &service_config)
    :  config_str(service_config)
      ,initialized(false)
      ,provided(0)
      ,mpi_rank(0)
      ,mpi_size(0)
  {
  }

  Faodel::Faodel()
    :  Faodel(DefaultFaodelConfig(default_pool_name))
      ,pool_name(default_pool_name)
  {
  }


  /**
   * Simple wrapper that uses mpisyncstart to setup dirman.
   */
  void Faodel::Bootstrap(int argc, char **argv) {
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    faodel::mpisyncstart::bootstrap();
  }

  /**
   * Simple wrapper that calls `Bootstrap` and uses the default configuration to call
   * the kelpie-specific bootstrap function.
   */
  void Faodel::BootstrapWithKelpie(int argc, char **argv) {
    Bootstrap(argc, argv);

    // create a dht named /myplace (see config) and connect to it
    faodel::bootstrap::Start(faodel::Configuration(config_str), kelpie::bootstrap);
  }

  /**
   * Simple wrapper that finishes the bootstrap and finalizes MPI.
   */
  void Faodel::Finish() {
    faodel::bootstrap::Finish();
    MPI_Finalize();
  }

  /**
   * Simple wrapper that connects to a kelpie pool.
   */
  auto Faodel::ConnectToPool() {
    return kelpie::Connect(pool_name);
  }

  /**
   * Simple wrapper that allocates a String object via lunasa.
   */
  auto Faodel::AllocateString(string &str_obj) {
    return lunasa::AllocateStringObject(str_obj);
  }

} // mohair::faodel

