// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "mohair/faodel.hpp"


// type aliases
using std::string;
using std::stringstream;

using std::endl;


// ------------------------------
// Module variables

// Default faodel configuration
namespace mohair::adapters {

  string DefaultFaodelConfig(const string &pool_name) {
    stringstream config_ss;

    config_ss << "# Use mpisyncstart to create a DHT (across all our nodes)" << endl
              << "# Name of dht is '/myplace'"                               << endl
              << "mpisyncstart.enable    true"                               << endl
              << "dirman.type            centralized"                        << endl
              << "dirman.root_node_mpi   0"                                  << endl
              << "dirman.resources_mpi[] dht:" << pool_name << " ALL"        << endl

              << "# Uncomment to get debug info for each component"          << endl
              // << "bootstrap.debug true"                                      << endl
              // << "dirman.debug    true"                                      << endl
              << "kelpie.debug    true"                                      << endl
    ;

    return config_ss.str();
  }

  Faodel::Faodel(const string &kpool_name, const string &service_config)
    :  config_str(service_config)
      ,pool_name(kpool_name)
      ,initialized(false)
      ,provided(0)
      ,mpi_rank(0)
      ,mpi_size(0)
  {
  }

  Faodel::Faodel(): Faodel(default_pool_name, DefaultFaodelConfig(default_pool_name)) {}


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
  kelpie::Pool Faodel::ConnectToPool() {
    return kelpie::Connect(pool_name);
  }

  /**
   * Simple wrapper that allocates a String object via lunasa.
   */
  lunasa::DataObject Faodel::AllocateString(const string &str_obj) {
    return lunasa::AllocateStringObject(str_obj);
  }

  /**
   * Wrapper that runs a lambda on a particular MPI rank.
   */
  void Faodel::FencedRankFn(int target_rank, std::function<void()> target_fn) {
    // start and end with a fence
    MPI_Barrier(MPI_COMM_WORLD);

    // execute the lambda on a particular rank
    if (mpi_rank == target_rank) { target_fn(); }

    MPI_Barrier(MPI_COMM_WORLD);
  }

} // mohair::faodel

