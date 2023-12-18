// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

// >> Configuration-based macros
#include "../mohair-config.hpp"

#if USE_FAODEL
  #include "../services/service_faodel.hpp"
#endif


// ------------------------------
// Functions
int ValidateArgs(int argc, char **argv) {
  if (argc != 1) {
    std::cerr << "Usage: faodel-service" << std::endl;
    return 1;
  }

  return 0;
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
  #if USE_FAODEL
    int validate_status = ValidateArgs(argc, argv);
    if (validate_status != 0) {
      std::cerr << "Failed to validate input command-line args" << std::endl;
      return validate_status;
    }

    // Start the flight service
    auto status_service = mohair::services::StartDefaultFaodelService();
    if (not status_service.ok()) {
      mohair::PrintError("Error running faodel service", status_service);
      return 2;
    }

  #else
    std::cerr << "Faodel feature is disabled." << std::endl;

  #endif

  return 0;
}
