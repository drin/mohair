#!/usr/bin/env python

# ------------------------------
# License

# Copyright 2023 Aldrin Montana
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# ------------------------------
# Module Docstring
"""
The plan definition and planning portion of query processing for decomposable queries.
"""


# ------------------------------
# Dependencies

# >> Standard libs
from dataclasses import dataclass

# >> Third-party libs
#   |> substrait types
from mohair.substrait.plan_pb2 import Plan

# >> Internal 
#   |> Logging
from mohair import CreateMohairLogger

#   |> classes
from mohair.query.types import MohairOp, MohairPlan, SuperPlan, SubPlan
from mohair.query.plans import PlanExplorer, PlanViewer
from mohair.query.decomposition import PlanSplitter

#   |> functions
from mohair.query.operators import MohairFrom


# ------------------------------
# Module Variables

# >> Logging
logger = CreateMohairLogger(__name__)


# ------------------------------
# Classes

@dataclass
class SubstraitPlan:
    """
    Wrapper class that references a serialized substrait plan and the root of its
    deserialized representation.
    """

    msg : bytes
    plan: Plan

    @classmethod
    def FromBytes(cls, plan_bytes: bytes, signed: bool=True) -> int:
        return int.from_bytes(plan_bytes, byteorder='big', signed=signed)

    @classmethod
    def ToBytes(cls, plan_hash: int, width: int=8, signed: bool=True) -> bytes:
        return plan_hash.to_bytes(width, byteorder='big', signed=signed)


# ------------------------------
# Functions (translation)

def ParseSubstrait(substrait_msg: bytes) -> SubstraitPlan:
    substrait_plan = Plan()
    substrait_plan.ParseFromString(substrait_msg)

    return SubstraitPlan(substrait_msg, substrait_plan)


def ParseQueryPlan(substrait_plan: SubstraitPlan) -> MohairPlan:
    mohair_root = None

    for plan_ndx, plan_root in enumerate(substrait_plan.plan.relations):
        logger.debug(f'translating plan {plan_ndx}')

        if plan_root.HasField('root'):
            # A query plan should have only 1 `root`, even if it has many trees
            assert mohair_root == None

            mohair_root = MohairFrom(plan_root.root.input)

    # Very confusing if a query plan did not have any `root`
    assert mohair_root is not None

    return PlanExplorer.WalkPlan(mohair_root)


def ViewQueryPlan(mohair_plan: MohairPlan) -> str:
    return PlanViewer.View(mohair_plan)


def ViewPlanOp(mohair_op: MohairOp) -> str:
    return PlanViewer.ViewOp(mohair_op)


def ViewQueryBreakers(mohair_plan: MohairPlan) -> str:
    return PlanViewer.ViewBreakers(mohair_plan)


def SplitQueryPlan(mohair_plan: MohairPlan) -> DecomposedPlan:
    return PlanSplitter.SplitPlan(mohair_plan)


# ------------------------------
# Main logic

from pathlib import Path

if __name__ == '__main__':
    # parse sample protobuf from a file, then translate it
    example_filepath = Path('resources') / 'examples' / 'average-expression.substrait'
    with open(example_filepath, 'rb') as file_handle:
        substrait_plan = ParseSubstrait(file_handle.read())

    mohair_plan = ParseQueryPlan(substrait_plan)
    plan_str    = ViewQueryPlan(mohair_plan, indent='  ')
    logger.debug(f'Mohair Plan:')
    logger.debug(plan_str)
