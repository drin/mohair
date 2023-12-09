// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "../mohair/mohair.hpp"
#include "../mohair/plans.hpp"
// #include "../engines/adapter_faodel.hpp"

#include <google/protobuf/text_format.h>


// >> Aliases
using mohair::QueryOp;
using mohair::AppPlan;

using google::protobuf::TextFormat;


// ------------------------------
// Functions
int ValidateArgs(int argc, char **argv) {
  if (argc != 2) {
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

  // Read the example substrait from a file
  auto file_stream    = mohair::StreamForFile(argv[1]);
  auto substrait_plan = mohair::SubstraitPlanFromFile(&file_stream);
  if (substrait_plan == nullptr) {
    std::cerr << "Failed to read substrait plan from file" << std::endl;
    return 2;
  }

  // Debug print substrait
  // PrintSubstraitPlan(substrait_plan.get());

  // Convert substrait to a plan we understand
  std::cout << "Parsing Substrait..." << std::endl;
  unique_ptr<QueryOp> mohair_root { mohair::MohairPlanFrom(substrait_plan.get()) };

  std::cout << "Traversing Mohair plan..." << std::endl;
  auto application_plan = mohair::FromPlanOp(mohair_root.get());
  /*
  if (application_plan == nullptr) {
    std::cerr << "Failed to parse substrait plan" << std::endl;
    return 10;
  }
  */

  std::cout << "Mohair Plan:"              << std::endl
            << application_plan->ViewPlan() << std::endl
  ;

  std::cout << "Breaker Leaves:" << std::endl;
  std::vector<unique_ptr<AppPlan>>& plan_bleaves = application_plan->bleaf_ops;
  for (size_t bleaf_ndx = 0; bleaf_ndx < plan_bleaves.size(); ++bleaf_ndx) {
    std::cout << "\t[" << std::to_string(bleaf_ndx) << "]" << std::endl;

    unique_ptr<AppPlan>& bleaf = plan_bleaves[bleaf_ndx];
    std::cout << bleaf->ViewPlan() << std::endl;
  }

  return 0;
}
