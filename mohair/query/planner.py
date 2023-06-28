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
import logging
import pdb

from functools   import singledispatch
from dataclasses import dataclass
from typing      import Any

# >> Substrait
from mohair.substrait.plan_pb2    import Plan
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


# >> Internal 
#   |> Logging
from mohair import AddConsoleLogHandler
from mohair import default_loglevel

#   |> classes
from mohair.mohair.algebra_pb2 import SkyRel, QueryRel
from mohair.query.operators import ( MohairPlan, MohairOp
                                    ,Projection, Selection, Aggregation, Join
                                    ,Read, SkyPartition
                                    ,PlanPipeline, PlanBreak
                                   )


# ------------------------------
# Module Variables

# >> Logging
logger = logging.getLogger(__name__)
logger.setLevel(default_loglevel)
AddConsoleLogHandler(logger)


# ------------------------------
# Classes

@dataclass
class QueryPlan:
    """
    Wrapper class that references the root of a substrait plan as well as the root of a
    mohair plan. The mohair plan only maintains references to operators in the substrait
    plan, so this class allows us to re-access the entirety of the substrait plan when
    necessary.
    """

    plan_msg      : bytes
    substrait_plan: Plan
    mohair_plan   : MohairPlan = None


    @classmethod
    def FromBytes(cls, plan_bytes: bytes) -> int:
        return MohairPlan.FromBytes(plan_bytes)

    @classmethod
    def ToBytes(cls, plan_hash: int) -> bytes:
        return MohairPlan.ToBytes(plan_hash)

    def __hash__(self):
        plan_hash = hash(self.mohair_plan)
        logger.debug(f'Hash of QueryPlan: {plan_hash}')
        return plan_hash


# ------------------------------
# Functions (translation)

@singledispatch
def MohairFrom(plan_op) -> Any:
    """
    A single-dispatch function that translates a substrait operator to a mohair operator.
    """

    raise NotImplementedError(f'No implementation for operator: {plan_op}')


def PrintMohairPlan(mohair_op: MohairOp, indent: str = '') -> Any:
    print(f'{indent}{mohair_op}')

    for input_op in mohair_op.input_ops:
        PrintMohairPlan(input_op, indent + '\t')


def TranslateSubstrait(substrait_msg: bytes) -> QueryPlan:
    """
    A convenience function that takes a serialized substrait plan (python bytes) and
    returns a `QueryPlan` instance.
    """

    substrait_plan = Plan()
    substrait_plan.ParseFromString(substrait_msg)

    mohair_plan = None
    for plan_ndx, plan_root in enumerate(substrait_plan.relations):
        logger.debug(f'translating plan {plan_ndx}')

        if plan_root.HasField('root'):
            # A query plan should have only 1 `root`, even if it has many trees
            assert mohair_plan == None

            # pdb.set_trace()
            mohair_plan = MohairFrom(plan_root.root.input)

    # Very confusing if a query plan did not have any `root`
    assert mohair_plan is not None

    return QueryPlan(substrait_msg, Plan(), mohair_plan)


# ------------------------------
# Dispatch functions for `MohairFrom`

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
    mohair_op = Read(read_op.base_schema, None, read_op)
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


# ------------------------------
# Main logic

if __name__ == '__main__':
    # parse sample protobuf from a file, then translate it
    with open('average-expression.substrait', 'rb') as file_handle:
        query_plan = TranslateSubstrait(file_handle.read())

    logger.debug(f'Mohair Plan:')
    # logger.debug(f'\t{query_plan.substrait_plan}')
    logger.debug(f'\t{query_plan.mohair_plan}')
