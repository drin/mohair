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
import logging

from typing import Any

from functools import singledispatch
from dataclasses import dataclass, field

# >> Internal

#   |> Logging
from mohair import AddConsoleLogHandler
from mohair import default_loglevel

#   |> Substrait definitions
#       |> relation types for common, leaf, unary, and N-ary relations
from mohair.substrait.algebra_pb2 import Rel
from mohair.substrait.algebra_pb2 import ReadRel, ExtensionLeafRel
from mohair.substrait.algebra_pb2 import (FilterRel,  FetchRel, AggregateRel, SortRel,
                                          ProjectRel, ExtensionSingleRel)
from mohair.substrait.algebra_pb2 import (JoinRel, SetRel, ExtensionMultiRel,
                                          HashJoinRel, MergeJoinRel)

#   |> Mohair definitions
#       |> leaf relation types
from mohair.mohair.algebra_pb2 import SkyRel, QueryRel


# ------------------------------
# Module Variables

# >> Logging
logger = logging.getLogger(__name__)
logger.setLevel(default_loglevel)
AddConsoleLogHandler(logger)


# ------------------------------
# Classes

# >> Operator structure

@dataclass
class MohairOp: pass

#   |> Unary relational classes
@dataclass
class Projection(MohairOp):
    plan_op: ProjectRel

    def __str__(self):
        return 'Projection()'

    def __hash__(self):
        return hash(self.__str__())

@dataclass
class Selection(MohairOp):
    plan_op: FilterRel

    def __str__(self):
        return 'Selection()'

    def __hash__(self):
        return hash(self.__str__())

@dataclass
class Aggregation(MohairOp):
    plan_op: AggregateRel

    def __str__(self):
        return 'Aggregation()'

    def __hash__(self):
        return hash(self.__str__())

@dataclass
class Limit(MohairOp):
    plan_op: FetchRel

    def __str__(self):
        return 'Limit()'

    def __hash__(self):
        return hash(self.__str__())


#   |> Leaf relational classes
@dataclass
class Read(MohairOp):
    plan_op: ReadRel
    name   : str = None

    def __post_init__(self):
        # grab the name of the table; otherwise just its type for now
        if self.plan_op.HasField('named_table'):
            self.name = '/'.join(self.plan_op.named_table.names)

        else:
            # TODO: we will eventually want a more robust name than the type of ReadRel
            self.name = self.plan_op.WhichOneof('read_type')

    def __str__(self):
        read_type = self.plan_op.WhichOneof('read_type')

        return f'Read({read_type})'

    def __hash__(self):
        return hash(self.__str__())

# NOTE: this should integrate a skytether partition and a SkyRel message
@dataclass
class SkyPartition(MohairOp):
    plan_op: SkyRel
    name   : str = None

    def __post_init__(self):
        self.name = f'{self.plan_op.domain}/{self.plan_op.partition}'

    def __str__(self):
        return f'SkyPartition({self.name})'

    def __hash__(self):
        return hash(self.__str__())


#   |>  Concrete relational classes (joins)
@dataclass
class Join(MohairOp):
    plan_op: JoinRel
    name   : str = None

    def __str__(self):
        return f'Join({self.name})'

    def __hash__(self):
        return hash(self.__str__())

@dataclass
class HashJoin(MohairOp):
    plan_op: HashJoinRel

    def __str__(self):
        return f'HashJoin()'

    def __hash__(self):
        return hash(self.__str__())

@dataclass
class MergeJoin(MohairOp):
    plan_op: MergeJoinRel

    def __str__(self):
        return f'MergeJoin()'

    def __hash__(self):
        return hash(self.__str__())


#   |>  Concrete relational classes (N-ary)
@dataclass
class SetOp(MohairOp):
    plan_op: SetRel

    def __str__(self):
        return f'SetOp()'

    def __hash__(self):
        return hash(self.__str__())


# >> High-level plan structure
@dataclass
class MohairPlan:

    @classmethod
    def FromBytes(cls, plan_bytes: bytes, signed: bool=True) -> int:
        return int.from_bytes(plan_bytes, byteorder='big', signed=signed)

    @classmethod
    def ToBytes(cls, plan_hash: int, width: int=8, signed: bool=True) -> bytes:
        return plan_hash.to_bytes(width, byteorder='big', signed=signed)


@dataclass
class PlanPipeline(MohairPlan):
    """
    Represents a holistic plan that has a list of pipelined ops that can be applied to the
    result of a subplan. If the subplan is None, then this should be a pipeline with a
    single data source as input.
    """

    op_pipeline: list[MohairOp]   = field(default_factory=list)
    name       : str              = None
    subplans   : list[MohairPlan] = field(default_factory=list)

    def __str__(self):
        return self.Print()

    def __hash__(self):
        return sum([hash(op) for op in self.op_pipeline])

    def Print(self, indent=''):
        return (
              f'{indent}PlanPipeline({self.name})\n'
            + f'{indent}[' + '\n\t'.join([str(op) for op in self.op_pipeline]) + ']'
            + f'\n{indent}|> subplans:\n'
            + '\n'.join([
                  subplan.Print(indent + '\t')
                  for subplan in self.subplans
              ])
        )

    def add_op(self, new_op: MohairOp):
        self.op_pipeline.append(new_op)
        return self

    def add_ops(self, new_ops: list[MohairOp]):
        self.op_pipeline.extend(new_ops)
        return self

@dataclass
class PlanBreak(MohairPlan):
    """
    Represents a holistic plan that has a list of pipelined ops that can be applied to the
    result of each subplan. This means that each subplan must be grouped first (or streamed
    over in some appropriate manner) and the pipeline operations can be applied to tuples
    as they flow through the result of the grouping operation (i.e. join or a set
    operator).
    """

    plan_op  : MohairOp
    name     : str              = None
    subplans : list[MohairPlan] = field(default_factory=list)

    def __post_init__(self):
        self.name = '.'.join([sub_root.name for sub_root in self.subplans])

    def __str__(self):
        return self.Print()

    def __hash__(self):
        return hash(self.plan_op) + sum([hash(subplan) for subplan in self.subplans])

    def Print(self, indent=''):
        return (
              f'{indent}PlanBreak({self.name}) <{self.plan_op}>\n'
            + f'{indent}>> subplans:\n'
            + '\n'.join([
                  subplan.Print(indent + '\t')
                  for subplan in self.subplans
              ])
        )


# ------------------------------
# Functions
@singledispatch
def MohairFrom(plan_op) -> Any:
    raise NotImplementedError(f'No implementation for operator: {plan_op}')

@MohairFrom.register
def _from_rel(plan_op: Rel) -> Any:
    """
    Translation function that propagates through the generic 'Rel' message.
    """

    op_rel = getattr(plan_op, plan_op.WhichOneof('rel_type'))
    # return MohairFrom(op_rel, plan_op)
    return MohairFrom(op_rel)

# >> Translations for unary relations
@MohairFrom.register
def _from_filter(filter_op: FilterRel) -> Any:
    logger.debug('translating <Filter>')

@MohairFrom.register
def _from_fetch(fetch_op: FetchRel) -> Any:
    logger.debug('translating <Fetch>')

@MohairFrom.register
def _from_sort(sort_op: SortRel) -> Any:
    logger.debug('translating <Sort>')

@MohairFrom.register
def _from_project(project_op: ProjectRel) -> Any:
    logger.debug('translating <Project>')

    mohair_subplan = MohairFrom(project_op.input)
    mohair_op      = Projection(project_op)

    if mohair_subplan is PlanPipeline:
        logger.debug('\t>> extending pipeline')

        mohair_subplan.add_op(mohair_op)
        return mohair_subplan

    return PlanPipeline([mohair_op], mohair_subplan.name, [mohair_subplan])


@MohairFrom.register
def _from_aggregate(aggregate_op: AggregateRel) -> Any:
    mohair_subplan = MohairFrom(aggregate_op.input)
    mohair_op      = Aggregation(aggregate_op)

    return PlanBreak(mohair_op, mohair_subplan.name, [mohair_subplan])


# >> Translations for leaf relations

@MohairFrom.register
def _from_readrel(read_op: ReadRel) -> Any:
    mohair_op = Read(read_op)
    return PlanPipeline([mohair_op], mohair_op.name)

@MohairFrom.register
def _from_skyrel(sky_op: SkyRel) -> Any:
    mohair_op = SkyPartition(sky_op)
    return PlanPipeline([mohair_op], mohair_op.name)

# >> Translations for join and n-ary relations

@MohairFrom.register
def _from_joinrel(join_op: JoinRel) -> Any:
    left_subplan    = MohairFrom(join_op.left)
    right_subplan   = MohairFrom(join_op.right)
    mohair_op       = Join(join_op)

    return PlanBreak(mohair_op, subplans=[left_subplan, right_subplan])
