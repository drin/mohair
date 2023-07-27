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
Convenience classes and functions for processing relational operators.
"""


# ------------------------------
# Dependencies

# >> Standard libs
from typing import Any, TypeAlias

from operator    import itemgetter, attrgetter
from functools   import singledispatch, singledispatchmethod
from dataclasses import dataclass, field

# >> Arrow
from pyarrow import Schema

# >> Internal
#   |> Logging
from mohair import CreateMohairLogger

#   |> Substrait definitions
from mohair.substrait.algebra_pb2 import (
         # top-level type
         Rel

         # leaf relations
        ,ReadRel
        ,ExtensionLeafRel

         # unary relations
        ,FilterRel
        ,FetchRel
        ,AggregateRel
        ,SortRel
        ,ProjectRel
        ,ExtensionSingleRel

         # n-ary relations
        ,JoinRel
        ,SetRel
        ,ExtensionMultiRel
        ,HashJoinRel
        ,MergeJoinRel
)

#       |> Mohair extensions
from mohair.mohair.algebra_pb2 import SkyRel, QueryRel

#   |> Internal classes
from mohair.query.types import MohairOp, PipelineOp, BreakerOp


# ------------------------------
# Module Variables

# >> Logging
logger = CreateMohairLogger(__name__)


# ------------------------------
# Classes

# >> Query operators that can stream data
@dataclass
class Projection(PipelineOp):
    plan_op : ProjectRel

@dataclass
class Selection(PipelineOp):
    plan_op : FilterRel

@dataclass
class Limit(PipelineOp):
    plan_op : FetchRel

@dataclass
class Sort(PipelineOp):
    plan_op : SortRel


# >> Leaf query operators (can stream data and are base cases)
@dataclass
class Read(PipelineOp):
    plan_op     : ReadRel
    op_inputs   : tuple[()] = ()

    def __post_init__(self):
        # grab the name of the table if it's available
        if self.plan_op.HasField('named_table'):
            self.table_name = '/'.join(self.plan_op.named_table.names)

        else:
            self.table_name = self.read_type()

    def __str__(self):
        op_name   = self.plan_op.DESCRIPTOR.name
        read_type = self.read_type()

        return f'{op_name}({read_type}:{self.table_name})'

    def read_type(self):
        return self.plan_op.WhichOneof('read_type')


# NOTE: this should integrate a skytether partition and a SkyRel message
@dataclass
class SkyPartition(PipelineOp):
    """
    A custom query operator to be used in a substrait query plan. This provides a way for
    the schema to be resolved at a remote query engine instead of having to know it up
    front.
    """

    plan_op     : SkyRel
    op_inputs   : tuple[()] = ()

    def __post_init__(self):
        dname = self.domain_key()
        pname = self.partition_key()

        self.table_name = f'{dname}/{pname}'

    def domain_key(self):
        return self.plan_op.domain

    def partition_key(self):
        return self.plan_op.partition


# >> Query operators that cannot stream data
@dataclass
class Aggregation(BreakerOp):
    plan_op : AggregateRel

@dataclass
class Join(BreakerOp):
    plan_op    : JoinRel
    op_inputs  : tuple[MohairOp, MohairOp]

@dataclass
class HashJoin(Join):
    plan_op: HashJoinRel

@dataclass
class MergeJoin(Join):
    plan_op: MergeJoinRel

@dataclass
class SetOp(BreakerOp):
    plan_op  : SetRel


# ------------------------------
# Functions for parsing a substrait query plan
@singledispatch
def MohairFrom(plan_op) -> Any:
    """
    Recursive function to parse the query plan. This handler executes if no other
    registered handler matches the argument type.
    """

    raise NotImplementedError(f'No implementation for operator: {plan_op}')


@MohairFrom.register
def _from_rel(plan_op: Rel) -> Any:
    """
    Translation function that propagates through the generic 'Rel' message.
    """

    op_rel = getattr(plan_op, plan_op.WhichOneof('rel_type'))
    return MohairFrom(op_rel)


# >> Translations for unary relations
@MohairFrom.register
def _from_project(project_op: ProjectRel) -> Any:
    logger.debug('translating <Project>')

    op_inputs = (MohairFrom(project_op.input),)
    return Projection(
         project_op, op_inputs
        ,table_name=op_inputs[0].table_name
    )


@MohairFrom.register
def _from_filter(filter_op: FilterRel) -> Any:
    logger.debug('translating <Filter>')

    op_inputs = (MohairFrom(filter_op.input),)
    return Selection(
         filter_op, op_inputs
        ,table_name=op_inputs[0].table_name
    )


@MohairFrom.register
def _from_fetch(fetch_op: FetchRel) -> Any:
    logger.debug('translating <Fetch>')

    op_inputs = (MohairFrom(fetch_op.input),)
    return Projection(
         fetch_op, op_inputs
        ,table_name=op_inputs[0].table_name
    )


@MohairFrom.register
def _from_sort(sort_op: SortRel) -> Any:
    logger.debug('translating <Sort>')

    op_inputs = (MohairFrom(sort_op.input),)
    return Projection(
         sort_op, op_inputs
        ,table_name=op_inputs[0].table_name
    )


@MohairFrom.register
def _from_aggregate(aggregate_op: AggregateRel) -> Any:
    logger.debug('translating <Aggregate>')

    op_inputs = (MohairFrom(aggregate_op.input),)
    return Aggregation(
         aggregate_op, op_inputs
        ,table_name=op_inputs[0].table_name
    )


# >> Translations for leaf relations
@MohairFrom.register
def _from_readrel(read_op: ReadRel) -> Any:
    logger.debug('translating <ReadRel>')

    return Read(read_op)

@MohairFrom.register
def _from_skyrel(sky_op: SkyRel) -> Any:
    logger.debug('translating <SkyRel>')

    return SkyPartition(sky_op)


# >> Translations for join and n-ary relations
@MohairFrom.register
def _from_joinrel(join_op: JoinRel) -> Any:
    logger.debug('translating <JoinRel>')

    op_inputs = (MohairFrom(join_op.left), MohairFrom(join_op.right))
    return Join(
         join_op, op_inputs
        ,table_name=':'.join(map(attrgetter('table_name'), op_inputs))
    )
