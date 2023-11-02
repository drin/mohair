// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "adapter_faodel.hpp"


// ------------------------------
// Type Aliases

//  >> Standard types
using std::endl;


// ------------------------------
// Functions

namespace mohair::adapters {

  // >> Static functions (not part of a class or struct)
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

} // namespace: mohair::adapters


// ------------------------------
// Classes and Methods

namespace mohair::adapters {
  //  >> Faodel adapter
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

  //  >> Convenience methods that interface with Faodel libraries
  /** Simple wrapper that registers a function. */
  void Faodel::RegisterEngineAcero() {
    std::cout << "Registering Execution Engine: Acero" << std::endl;
    kelpie::RegisterComputeFunction("ExecuteEngineAcero", mohair::adapters::ExecuteSubstrait);
  }

  /** Simple wrapper that connects to a kelpie pool. */
  KelpPool Faodel::ConnectToPool() { return kelpie::Connect(pool_name); }

  /** Simple wrapper that allocates a String object via lunasa. */
  LunaDO Faodel::AllocateString(const string &str_obj) {
    return lunasa::AllocateStringObject(str_obj);
  }

  void Faodel::PublishTable(const shared_ptr<Table> &data, KelpPool &kpool, KelpKey &kkey) {
    // NOTE: arrow::Compression is in arrow/util/type_fwd which is transitively imported
    // by arrow/ipc/api.h. For now, just store the Table uncompressed (this tells faodel
    // how to store it, not how to access it)
    ArrowDO fado { data, arrow::Compression::UNCOMPRESSED };

    kpool.Publish(kkey, fado.ExportDataObject());
  }

  Result<shared_ptr<Table>>
  Faodel::ExecuteEngineAcero(KelpPool &kpool, KelpKey &kkey, const shared_ptr<Buffer> &plan_msg) {
    // Execute the compute function and put the result in `ldo_result`
    LunaDO ldo_result;
    kpool.Compute(kkey, "ExecuteEngineAcero", plan_msg->ToString(), &ldo_result);

    // Wrap the faodel result in an arrow data object (faodel-lunasa -> faodel-arrow)
    ArrowDO fado_result { ldo_result };
    const auto table_count = fado_result.NumberOfTables();

    // Prepare a vector to store each table result
    std::vector<shared_ptr<Table>> table_list;
    table_list.reserve(table_count);

    // Walk each table in the faodel object and extract each into the vector
    for (int table_ndx = 0; table_ndx < table_count; ++table_ndx) {
      ARROW_ASSIGN_OR_RAISE(auto fado_subtable, fado_result.ExtractTable(table_ndx));
      table_list.emplace_back(fado_subtable);
    }

    return arrow::ConcatenateTables(table_list);
  }

  //  >> Convenience methods that interface with MPI
  /** Simple wrapper that uses mpisyncstart to setup dirman. */
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

  /** Simple wrapper that finishes the bootstrap and finalizes MPI. */
  void Faodel::Finish() {
    faodel::bootstrap::Finish();
    MPI_Finalize();
  }

  /** Simple wrapper that prints our MPI rank and the size of the MPI pool. */
  void Faodel::PrintMPIInfo() {
    std::cout << "\tMPI Size: " << std::to_string(mpi_size) << std::endl
              << "\tMPI rank: " << std::to_string(mpi_rank) << std::endl
    ;
  }

  /** Wrapper that runs a lambda on a particular MPI rank. */
  void Faodel::FencedRankFn(int target_rank, std::function<void()> target_fn) {
    // start and end with a fence
    MPI_Barrier(MPI_COMM_WORLD);

    // execute the lambda on a particular rank
    if (mpi_rank == target_rank) { target_fn(); }

    MPI_Barrier(MPI_COMM_WORLD);
  }


  //  >> Methods for Acero integration

  /**
   * Convenience function that calls `ProviderForFadoMap` and passes the member fado_map.
   *
   * This was defined first, but example code did not use this Faodel class, so this
   * approach is for transitional purposes.
   */
  NamedTableProvider Faodel::FadoTableProvider() {
    return mohair::adapters::ProviderForFadoMap(this->fado_map);
  }

  // end of Faodel class functions

} // namespace mohair::adapters

