// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "faodel.hpp"


// ------------------------------
// Functions
void PrintStringObj(const std::string print_msg, const std::string string_obj) {
  std::cout << "Printing string obj:"  << std::endl;
  std::cout << print_msg << string_obj << std::endl;
}

// Calls Kelpie's built-in function, 'pick'.
// Pick has four options for arguments: 'first', 'last','smallest', and 'biggest'.
// This function returns the object with key name that is first or last in the wildcard
// list (alphabetically); or, that is smallest or largest in size.
void CallPick( kelpie::Pool       &kpool
              ,kelpie::Key        &kname
              ,const std::string   fn_arg
              ,lunasa::DataObject *result) {
  // TODO: could validate fn_arg is one of 'first', 'last', 'smallest', 'biggest'.
  // key_name may have a wildcard suffix
  std::cout << "Calling compute function 'pick'" << std::endl;
  kpool.Compute(kname, "pick", fn_arg, result);
}


int main(int argc, char **argv) {
  Faodel faodel_adapter;

  faodel_adapter.BootstrapWithKelpie(argc, argv);
  std::cout << "\tMPI Size: " << std::to_string(mpi_size) << std::endl
            << "\tMPI rank: " << std::to_string(mpi_rank) << std::endl
  ;

  // >> Things that happen on our rank

  // default pool is "/myplace"
  auto pool = faodel_adapter.ConnectToPool();
  auto ldo1 = faodel_adapter.AllocateString(
      "This is an object from rank "
    + std::to_string(mpi_rank)
    // + std::string { std::to_string(mpi_size - mpi_rank) + '!' }
    + std::string(mpi_size - mpi_rank, '!')
  );

  // Publish string object to pool
  kelpie::Key k1 {"myrow", std::to_string(mpi_rank)};
  pool.Publish(k1, ldo1);

  // Wait for everyone to be done
  MPI_Barrier(MPI_COMM_WORLD);

  // work restricted to rank 0
  if (mpi_rank == 0) {
    // '*' is a wildcard suffix for "any column with same prefix"
    kelpie::Key key_myrow("myrow", "*"); 

    lunasa::DataObject ldo_first, ldo_last, ldo_smallest, ldo_largest;
    CallPick(pool, key_myrow, "first"   , &ldo_first   );
    CallPick(pool, key_myrow, "last"    , &ldo_last    );
    CallPick(pool, key_myrow, "smallest", &ldo_smallest);
    CallPick(pool, key_myrow, "largest" , &ldo_largest );

    // Should be from rank 0
    PrintStringObj("First item:    ", lunasa::UnpackStringObject(ldo_first));

    // Should be from last rank
    PrintStringObj("Last item:     ", lunasa::UnpackStringObject(ldo_last)    );
    PrintStringObj("Smallest item: ", lunasa::UnpackStringObject(ldo_smallest));
    PrintStringObj("Largest item:  ", lunasa::UnpackStringObject(ldo_largest) );
  }

  MPI_Barrier(MPI_COMM_WORLD);

  faodel_adapter.Finish();

  return 0;
}
