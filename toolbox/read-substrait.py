# ------------------------------
# Dependencies

# >> Standard modules
import sys

from pathlib import Path

# >> Third-party
import pyarrow
import pandas
import ibis

from ibis_substrait.compiler.core import SubstraitCompiler

from mohair.substrait.plan_pb2 import Plan
from mohair.substrait.algebra_pb2 import Rel


# ------------------------------
# Module variables

example_input = Path('resources') / 'examples' / 'average-expression.substrait'


if __name__ == '__main__':
    # initialize a protobuf structure
    substrait_plan = Plan()

    # populate the protobuf structure from binary
    plan_message_fpath = Path(sys.argv[1]) or example_input
    with open(plan_message_fpath, 'rb') as file_handle:
        print(f'Source of plan: {plan_message_fpath}')
        substrait_plan.ParseFromString(file_handle.read())

    # print the structure
    print(substrait_plan)
