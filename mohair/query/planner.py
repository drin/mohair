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


# >> Internal 
#   |> Logging
from mohair import CreateMohairLogger

#   |> classes
from mohair.query.types import MohairOp, MohairPlan, DecomposedPlan
from mohair.query.substrait import SubstraitPlan
from mohair.query.plans import PlanExplorer, PlanViewer
from mohair.query.decomposition import PlanSplitter

#   |> functions
from mohair.query.operators import MohairFrom


# ------------------------------
# Module Variables

# >> Logging
logger = CreateMohairLogger(__name__)


# ------------------------------
# Functions

def ViewQueryPlan(mohair_plan: MohairPlan) -> str:
    return PlanViewer.View(mohair_plan)


def ViewPlanOp(mohair_op: MohairOp, indent='') -> str:
    return PlanViewer.ViewOp(mohair_op, indent)


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
        substrait_plan = SubstraitPlan.FromMessageBytes(file_handle.read())

    mohair_plan = substrait_plan.ToMohair()
    plan_str    = ViewQueryPlan(mohair_plan, indent='  ')
    logger.debug(f'Mohair Plan:')
    logger.debug(plan_str)
