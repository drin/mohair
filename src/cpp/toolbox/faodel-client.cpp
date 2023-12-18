// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

// >> Configuration-based macros
#include "../mohair-config.hpp"

#include "../services/service_faodel.hpp"


// ------------------------------
// Functions
int ValidateArgs(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: faodel-client <path-to-plan-file>" << std::endl;
    return 1;
  }

  return 0;
}

shared_ptr<Buffer> ReadSubstraitFromFile(const char *plan_fpath) {
  std::cout << "Reading plan from: '" << plan_fpath  << "'"
            << std::endl
  ;

  string plan_msg;
  if (not mohair::FileToString(plan_fpath, plan_msg)) {
    std::cerr << "Failed to parse file data into string" << std::endl;
    return nullptr;
  }

  return Buffer::FromString(plan_msg);
}

int SendMohairRequest(const char* plan_fpath) {
  // >> Using CLI args, grab a substrait plan
  auto substrait_plan = ReadSubstraitFromFile(plan_fpath);
  if (substrait_plan == nullptr) {
    std::cerr << "Failed to read substrait plan from file" << std::endl;
    return 2;
  }

  // >> Connect to the mohair service
  // grab the default service location
  Location service_loc;
  auto status_loc = Location::ForGrpcTcp("0.0.0.0", 40847, &service_loc);
  if (not status_loc.ok()) {
    mohair::PrintError("Error getting service location", status_loc);
    return 3;
  }

  // Get a client connected to the flight service
  auto result_client = FlightClient::Connect(service_loc);
  if (not result_client.ok()) {
    mohair::PrintError("Error connecting to service", result_client.status());
    return 4;
  }

  unique_ptr<FlightClient> mohair_client = std::move(result_client).ValueOrDie();

  // >> Now give work to the mohair service
  FlightCallOptions flight_opts;
  Action            query_action { "query", substrait_plan };

  auto result_stream = mohair_client->DoAction(flight_opts, query_action);
  if (not result_stream.ok()) {
    mohair::PrintError("Error executing substrait plan", result_stream.status());
    return 5;
  }

  unique_ptr<ResultStream> query_results = std::move(result_stream).ValueOrDie();
  auto result_response = query_results->Next();
  do {
    if (not result_response.ok()) {
      mohair::PrintError("Execution error.", result_response.status());
      return 6;
    }

    std::cout << "parsed successful results" << std::endl;

    result_response = query_results->Next();
  }
  while (result_response.ok());

  std::cout << "Query results:" << std::endl;
}


// ------------------------------
// Main Logic
int main(int argc, char **argv) {
  int validate_status = ValidateArgs(argc, argv);
  if (validate_status != 0) {
    std::cerr << "Failed to validate input command-line args" << std::endl;
    return validate_status;
  }

  return SendMohairRequest(argv[1]);
}
