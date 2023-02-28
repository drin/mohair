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
The plan definition and planning portion of query processing for decomposable queries.
"""


# ------------------------------
# Dependencies

# >> Standard libs
from functools import singledispatch
from dataclasses import dataclass

# >> Third-party libs

#   |> substrait types
from mohair.substrait.plan_pb2 import Plan

from mohair.substrait.algebra_pb2 import Rel
from mohair.substrait.algebra_pb2 import ( ReadRel  , ExtensionLeafRel
                                          
                                          # unary relations
                                          ,FilterRel,  FetchRel, AggregateRel, SortRel
                                          ,ProjectRel, ExtensionSingleRel

                                          # n-ary relations
                                          ,JoinRel, SetRel, ExtensionMultiRel
                                          ,HashJoinRel, MergeJoinRel
                                         )

# >> Internal 

#   |> classes
from mohair.query.operators import MohairPlan

#   |> functions
from mohair.query.operators import MohairFrom


# ------------------------------
# Classes

@dataclass
class QueryPlan:
    """
    """

    substrait_plan: Plan
    mohair_plan   : MohairPlan = None



# ------------------------------
# Functions (translation)

def TranslateSubstrait(substrait_plan: Plan) -> QueryPlan:
    mohair_plan = None

    for plan_ndx, plan_root in enumerate(substrait_plan.relations):
        print(f'translating plan {plan_ndx}')

        if plan_root.HasField('root'):
            # A query plan should have only 1 `root`, even if it has many trees
            assert mohair_plan == None

            mohair_plan = QueryPlan(
                 substrait_plan
                ,MohairFrom(plan_root.root.input)
            )

    # Very confusing if a query plan did not have any `root`
    assert mohair_plan is not None

    return mohair_plan



# ------------------------------
# Main logic

if __name__ == '__main__':
    # Declare plan
    test_plan = Plan()

    # Populate plan from sample protobuf
    with open('average-expression.substrait', 'rb') as file_handle:
        test_plan.ParseFromString(file_handle.read())

    # Convert protobuf into a structure we can process
    query_plan = TranslateSubstrait(test_plan)

    print(f'Mohair Plan:')
    # print(f'\t{query_plan.substrait_plan}')
    print(f'\t{query_plan.mohair_plan}')
