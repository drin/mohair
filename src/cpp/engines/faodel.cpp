// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "../headers/adapter_faodel.hpp"


// Type Aliases
using std::string;
using std::stringstream;

using std::endl;


// ------------------------------
// Module variables


// ------------------------------
// Classes and Functions

// >> Convenience functions
string JoinStr(vector<string> str_parts, const char *delim) {
  stringstream join_stream;

  join_stream << str_parts[0];
  for (int ndx = 1; ndx < str_parts.size(); ++ndx) {
    join_stream << delim << str_parts[ndx];
  }

  return std::move(join_stream.str());
}


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


  /**
   * Convenience higher-order function that returns a `NamedTableProvider`.
   *
   * A NamedTableProvider is a function that takes 2 parameters and returns a
   * `Declaration`.
   *
   * The 2 parameters describe a requested table:
   *  - a decomposed table name (vector<string> that represents a single table)
   *  - a table schema
   *
   * The resulting Declaration describes a Source Node for a query plan.
   */
  NamedTableProvider ProviderForFadoMap(map<KelpKey, LunaDO> &fado_map) {
    /**
     * A lambda that captures the given fado_map by reference and takes two parameters:
     *  - tname  : a vector of strings that collectively make up a single table name
     *  - tschema: an expected schema of the table, only used with early binding
     */
    return [&fado_map]( const vector<string> &tname
                       ,const Schema         &tschema) -> Result<Declaration> {

      // gather the parts of the table name
      auto requested_tname = JoinStr(tname, ".");

      // lookup the name in the fado_map, store entry into a structured binding
      if (not fado_map.contains(requested_tname)) {
        return arrow::Status::KeyError(
           "Fado table provider could not find table: [", requested_tname, "]"
        );
      }

      // wrap the lunasa data object in a faodel arrow data object
      auto [&key_name, &key_val] = fado_map[requested_tname];
      ArrowDO fado { key_val };

      vector<shared_ptr<Table>> fado_chunks;
      fado_chunks.reserve(fado.NumberOfTables());

      // extract each table from the data object (also called chunks)
      for (int table_ndx = 0; table_ndx < fado.NumberOfTables(); ++table_ndx) {
        ARROW_ASSIGN_OR_RAISE(auto fado_chunk, fado.ExtractTable(table_index));
        fado_chunks.push_back(fado_chunk);
      }

      // concatenate into a single table which we wrap in a Declaration
      // the Declaration essentially represents a scan node
      ARROW_ASSIGN_OR_RAISE(auto fado_as_table, arrow::ConcatenateTables(fado_chunks));
      auto options = arrow::acero::TableSourceNodeOptions(std::move(fado_as_table));

      return Declaration<TableSourceNodeOptions>(
         "table_source"
        ,TableSourceNodeOptions { std::move(fado_as_table) }
        ,requested_tname
      );
    };
  }
  

  // >> Faodel class functions

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


  // ------------------------------
  // Functions for interfacing with Faodel libraries

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


  // ------------------------------
  // Functions for MPI integration

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
   * Simple wrapper that prints our MPI rank and the size of the MPI pool.
   */
  void Faodel::PrintMPIInfo() {
    std::cout << "\tMPI Size: " << std::to_string(mpi_size) << std::endl
              << "\tMPI rank: " << std::to_string(mpi_rank) << std::endl
    ;
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


  // ------------------------------
  // Functions for Arrow and Acero integration

  /**
   * Convenience function that calls `ProviderForFadoMap` and passes the member fado_map.
   *
   * This was defined first, but example code did not use this Faodel class, so this
   * approach is for transitional purposes.
   */
  NamedTableProvider Faodel::FadoTableProvider() {
    return ProviderForFadoMap(this->fado_map);
  }

  // end of Faodel class functions

} // namespace mohair::adapters

