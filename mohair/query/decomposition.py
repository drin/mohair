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
Classes and functions for decomposing query plans for delegation and composing pushback
plans.
"""


# ------------------------------
# Dependencies

# >> Standard libs
from typing import Any
from operator import attrgetter

from enum import Enum

# >> Internal
from mohair import CreateMohairLogger
from mohair.query.types import MohairOp, PipelineOp, BreakerOp
from mohair.query.types import MohairPlan, DecomposedPlan


# ------------------------------
# Module Variables

# >> Logging
logger = CreateMohairLogger(__name__)


# ------------------------------
# Classes

# >> For splitting a MohairPlan

class SplitOpt(Enum):
    """ Enumeration of algorithms for splitting a query plan. """

    LongestChain  = 1
    MiddleBreaker = 2
    LowestBreaker = 3
    ByWeight      = 4

def_split_method = SplitOpt.LongestChain
class PlanSplitter:
    """ Methods for splitting a query plan; coalesced as a visitor class. """

    @classmethod
    def SplitPlan(cls, plan: MohairPlan, method: SplitOpt=def_split_method) -> DecomposedPlan:
        """
        A function to decompose a `MohairPlan` into two portions: a `SuperPlan` and a
        `SubPlan`. The SubPlan is multicast to downstream devices that:
        -   may have the data
        -   may handle the work

        Each pushback plan received updates our view of a downstream device's:
        -   available data (dynamic catalog)
        -   compute capabilities

        The particular algorithm is specified by :method:.
        """

        if method is SplitOpt.LongestChain:
            split_plan_root = cls.FindLongestChain(plan)

        sub_roots = split_plan_root.plan_root.op_inputs
        return DecomposedPlan(plan, split_plan_root, sub_roots)

    @classmethod
    def FindLongestChain(cls, plan: MohairPlan) -> MohairPlan:
        """
        Method to find the 'longest chain' in a `MohairPlan`. A chain is a sequence of
        unary operators, or a query plan with only 1 leaf.

        This method returns a subplan with 2 or fewer leaves and the largest height.
        """

        # Default to breaker leaf with longest pipeline
        longest_leaf = max(plan.breaker_leaves, key=attrgetter('pipeline_len'))

        # But find longest linear chain (non-bushy tree)
        longest_chain = max(
             filter(lambda plan_op: plan_op.plan_width <= 2, plan.breaker_list)
            ,default=longest_leaf
            ,key=attrgetter('plan_height')
        )

        return longest_chain
