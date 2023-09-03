// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "../headers/faodel.hpp"

using mohair::adapters::Faodel;


// ------------------------------
// Functions

void PrintStringObj(const std::string print_msg, const std::string string_obj) {
  std::cout << "Printing string obj:"  << std::endl;
  std::cout << print_msg << string_obj << std::endl;
}

void MPIMain(int argc, char **argv) {
  Faodel faodel_adapter;

  faodel_adapter.BootstrapWithKelpie(argc, argv);
  faodel_adapter.PrintMPIInfo();

  // A for loop to round robin through each rank.
  for (int rank_id = 0; rank_id < faodel_adapter.mpi_size; ++rank_id) {

    // This is to progressively debug more steps
    // TODO: build towards a linear chain of ranks that execute and pass pushforward plans
    if (rank_id >= 0) { break; }
  }

  // TODO
  // >> Define the object to put in the pool
  auto ldo1 = faodel_adapter.AllocateString(testobj_stream.str());
  kelpie::Key k1 {"myrow", std::to_string(faodel_adapter.mpi_rank)};

  // >> publish key-value pair to pool (default is "/myplace")
  auto pool = faodel_adapter.ConnectToPool();
  pool.Publish(k1, ldo1);

  auto sample_fn = [kpool=std::move(pool)]() mutable noexcept {
    // '*' is a wildcard suffix for "any column with same prefix"
    kelpie::Key key_myrow("myrow", "*"); 

    lunasa::DataObject ldo_first, ldo_last, ldo_smallest, ldo_largest;
  };

  // execute `sample_fn` on rank 0
  // this also adds barriers around the lambda
  faodel_adapter.FencedRankFn(/*mpi_rank==*/0, sample_fn);

  faodel_adapter.Finish();
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
  std::cout << argv[0] << std::endl;

  return 0;
}