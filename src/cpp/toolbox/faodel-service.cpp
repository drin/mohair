// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "../services/service_faodel.hpp"

#include <google/protobuf/text_format.h>


// >> Aliases
using google::protobuf::TextFormat;


// ------------------------------
// Functions
int ValidateArgs(int argc, char **argv) {
  if (argc != 1) {
    std::cerr << "Usage: mohair <path-to-substrait-plan>" << std::endl;
    return 1;
  }

  return 0;
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
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

  return 0;
}
