# ------------------------------
# Dependencies

# >> Standard modules
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
    with open(example_input, 'rb') as file_handle:
        substrait_plan.ParseFromString(file_handle.read())

    # print the structure
    print(substrait_plan)
