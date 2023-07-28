#!/usr/bin/env python

# ------------------------------
# License

# Copyright 2022 Aldrin Montana
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
Convenience classes and functions for wrapping substrait and mohair types.
"""


# ------------------------------
# Dependencies

# >> Standard libs
from operator import attrgetter
from dataclasses import dataclass, field
from typing import Any, TypeAlias

# >> Internal
from mohair import CreateMohairLogger


# ------------------------------
# Module Variables

# >> Logging
logger = CreateMohairLogger(__name__)

# >> Forward references (Type aliases)
OpTypeAlias  : TypeAlias = 'MohairOp'
PlanTypeAlias: TypeAlias = 'MohairPlan'


# ------------------------------
# Classes

# >> Operator base classes
@dataclass
class MohairOp:
    """
    A single operator in a query plan that primarily maintains references to operators in
    the substrait plan.
    """

    plan_op   : Any
    op_inputs : tuple[OpTypeAlias, ...]
    table_name: str = ''

    def __str__(self):
        op_name = self.plan_op.DESCRIPTOR.name
        return f'{op_name}({self.table_name})'

@dataclass
class PipelineOp(MohairOp):
    """ A query operator that tuples can stream through. """

    def ViewStr(self): return f'← {self}'


@dataclass
class BreakerOp(MohairOp):
    """ A stateful query operator that tuples cannot naively stream through. """

    def ViewStr(self): return f'↤ {self}'


# >> Graph classes

@dataclass
class MohairPlan:
    """
    Query plan that propagates through a computational storage system. Also stores
    properties of the plan for later analysis.

    :plan_root:      The `MohairOp` that is the root of this plan.
    :pipeline_len:   The length of the longest pipeline.
    :breaker_height: The max count of pipeline breakers in the plan.
    :breaker_count:  The total count of breakers in the plan.
    :breaker_leaves: The "bottom-most" pipeline breakers in the query plan.
    """

    plan_root: MohairOp

    pipeline_len  : int = 0
    plan_width    : int = 0
    plan_height   : int = 0
    breaker_count : int = 0
    breaker_height: int = 0
    breaker_leaves: list[PlanTypeAlias] = field(default_factory=list)
    breaker_list  : list[PlanTypeAlias] = field(default_factory=list)

    def __hash__(self):
        plan_hash = hash(self.plan_root)
        logger.debug(f'Hash of MohairPlan: {plan_hash}')

        return plan_hash

    def IncrementPipelineLength(self) -> int:
        """
        Function to get an incremented pipeline length. If this plan is rooted with a
        pipeline-able operator, then :pipeline_len: is incremented, otherwise 1 is
        returned.
        """

        if issubclass(type(self.plan_root), BreakerOp): return 1
        return self.pipeline_len + 1

@dataclass
class DecomposedPlan:
    """ Portion of a query plan that is *not* delegated downstream. """

    query_plan     : MohairPlan
    anchor_root    : MohairPlan
    anchor_subplans: tuple[MohairOp, ...]


@dataclass
class LogicalExecPlan(MohairPlan):
    pass

