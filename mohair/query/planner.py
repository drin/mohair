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
from typing      import Any


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


def ViewPlanOp(mohair_op: MohairOp) -> str:
    return PlanViewer.ViewOp(mohair_op)


def ViewQueryBreakers(mohair_plan: MohairPlan) -> str:
    return PlanViewer.ViewBreakers(mohair_plan)


def SplitQueryPlan(mohair_plan: MohairPlan) -> DecomposedPlan:
    return PlanSplitter.SplitPlan(mohair_plan)


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
