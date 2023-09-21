// ------------------------------
// License

// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.


// ------------------------------
// Dependencies

#include "../headers/mohair.hpp"
#include "../headers/adapter_faodel.hpp"

#include "../mohair/util.hpp"

#include <google/protobuf/text_format.h>


// >> Aliases
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

unique_ptr<PlanRel> ReadSubstraitFromFile(const char *plan_fpath) {
  std::cout << "Reading plan from: '" << plan_fpath  << "'"
            << std::endl
  ;

  string plan_msg;
  if (not FileToString(plan_fpath, plan_msg)) {
    std::cerr << "Failed to parse file data into string" << std::endl;
    return nullptr;
  }

  return SubstraitPlanFromString(plan_msg);
}

int StringForSubstrait(unique_ptr<PlanRel> &substrait_plan, string *plan_text) {
  auto success = TextFormat::PrintToString(*substrait_plan, plan_text);
  if (not success) {
    std::cerr << "Unable to print substrait plan as string" << std::endl;
    return 3;
  }

  return 0;
}

int ExecWithFaodel(int argc, char **argv, unique_ptr<PlanRel> substrait_plan) {
  mohair::adapters::Faodel faodel_adapter;

  // bootstrap and display configuration
  faodel_adapter.BootstrapWithKelpie(argc, argv);
  faodel_adapter.PrintMPIInfo();

  // register a compute function with kelpie
  kelpie::RegisterComputeFunction("ExecuteWithAcero", mohair::adapters::ExecuteSubstrait);

  // TODO:
  // execute `sample_fn` on rank 0
  // this also adds barriers around the lambda
  // faodel_adapter.FencedRankFn(/*mpi_rank==*/0, QueryEngineMain());

  faodel_adapter.Finish();

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

  auto substrait_plan = ReadSubstraitFromFile(argv[1]);
  if (substrait_plan == nullptr) {
    std::cerr << "Failed to read substrait plan from file" << std::endl;
    return 2;
  }

  string plan_text;
  auto stringify_status = StringForSubstrait(substrait_plan, &plan_text);
  if (stringify_status != 0) {
    std::cerr << "Failed to stringify substrait plan" << std::endl;
    return stringify_status;
  }
  std::cout << "Substrait Plan:" << std::endl
            << plan_text        << std::endl
  ;

  // Delegate faodel work
  return ExecWithFaodel(argc, argv, std::move(substrait_plan));

  return 0;
}
