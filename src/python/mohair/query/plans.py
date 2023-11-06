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
Classes and functions for interacting with query plans.
"""


# ------------------------------
# Dependencies

# >> Standard libs
from typing import Any

# >> Internal
from mohair import CreateMohairLogger
from mohair.query.types import MohairOp, PipelineOp, BreakerOp
from mohair.query.types import MohairPlan

# ------------------------------
# Module Variables

# >> Logging
logger = CreateMohairLogger(__name__)


# ------------------------------
# Classes

class PlanViewer:
    """ Class for printing or viewing a query plan. """

    @classmethod
    def View(cls, query_plan: MohairPlan, indent='') -> str:
        """ Function to view a plan. """

        logger.debug('Viewing Mohair plan')
        return cls.ViewOp(query_plan.plan_root, indent)

    @classmethod
    def ViewOp(cls, query_op: MohairOp, indent='') -> str:
        """ Function to recursively view operators in a plan. """

        op_prefix = '  '
        if issubclass(type(query_op), BreakerOp): op_prefix = f'\n{indent}'

        input_view_str = cls.ViewInputOps(query_op, indent)
        op_str         = query_op.ViewStr()

        return f'{op_prefix}{op_str}{input_view_str}'

    @classmethod
    def ViewInputOps(cls, query_op: MohairOp, indent='') -> str:
        """ Convenience function to recurse on operator inputs. """

        return ''.join([
            cls.ViewOp(input_op, indent + '  ')
            for input_op in query_op.op_inputs
        ])

    @classmethod
    def ViewBreakers(cls, query_plan: MohairPlan, indent='') -> str:
        """ Function to view the pipeline breakers in a plan. """

        logger.debug('Viewing pipeline breakers')

        # First, the leaves
        # print(type(query_plan.breaker_leaves[0].plan_root))
        # return 'test'
        leaf_breakers = f'\n{indent}'.join([
            str(breaker_op.plan_root)
            for breaker_op in query_plan.breaker_leaves
        ])

        return f'{indent}{leaf_breakers}'


# >> Functions for walking a query plan
class PlanExplorer:

    @classmethod
    def WalkPlan(cls, plan_op: MohairOp) -> MohairPlan:
        """ Function to build a `MohairPlan` pointing to :plan_op:. """

        # Gather a list of ordered `MohairPlan`s for input ops
        input_plans = cls.WalkInputOps(plan_op)
        
        # Compute properties for this `MohairPlan`
        mohair_plan = MohairPlan(plan_op)

        # For everything else we need to compare subplans
        pipeline_len   = 1
        plan_width     = 0
        plan_height    = 0
        breaker_height = 0
        breaker_count  = 0
        breaker_leaves = []  # maybe see if changing these to tuples improves perf.
        breaker_list   = []

        for input_plan in input_plans:
            # plan width (leaf count) is a simple sum
            plan_width  += input_plan.plan_width
            plan_height  = max(plan_height, input_plan.plan_height)

            # breaker count is a simple sum; breaker height is max of breaker heights
            breaker_count  += input_plan.breaker_count
            breaker_height  = max(breaker_height, input_plan.breaker_height)

            # if a subplan has a breaker, it is still a leaf for us
            breaker_leaves.extend(input_plan.breaker_leaves)

            # maintain an iterable list of breakers for easy access
            breaker_list.extend(input_plan.breaker_list)

        # include this operator in properties of the plan
        if issubclass(type(plan_op), BreakerOp):
            breaker_count  += 1
            breaker_height += 1

            # if no other leaves, we are a leaf
            if not breaker_leaves: breaker_leaves.append(mohair_plan)

            # otherwise, we're just a breaker
            else:                  breaker_list.append(mohair_plan)

        # if we had any inputs at all, they're sorted by pipeline length
        if input_plans: pipeline_len = input_plans[0].IncrementPipelineLength()

        # Set plan properties
        mohair_plan.pipeline_len   = pipeline_len
        mohair_plan.plan_width     = plan_width or 1
        mohair_plan.plan_height    = plan_height + 1
        mohair_plan.breaker_count  = breaker_count
        mohair_plan.breaker_height = breaker_height
        mohair_plan.breaker_leaves = breaker_leaves
        mohair_plan.breaker_list   = breaker_list

        return mohair_plan


    @classmethod
    def WalkInputOps(cls, query_op: MohairOp) -> list[MohairPlan]:
        """ Convenience function to recurse on operator inputs. """

        # recurse on input operators
        input_plans = map(cls.WalkPlan, query_op.op_inputs)

        # sort subplans by calling the `IncrementPipelineLength` method
        # this prioritizes extendable pipelines when there are ties in length
        def GetPipeLen(mohair_plan):
            return mohair_plan.IncrementPipelineLength()

        return sorted(input_plans, key=GetPipeLen)

