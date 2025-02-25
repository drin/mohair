#!/usr/bin/env python

# ------------------------------
# License

# Copyright 2024 Aldrin Montana
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
# Overview
"""
Ibis code for queries that compute statistics.

These queries are specific tasks that fall under the workload category, "Compute
Statistics" (W4). These types of queries compute some numerical value representing a
summary statistic that is most commonly going to be held in memory or as part of a pinned
working set (materialized view). Additionally, these queries will frequently be reused as
part of other types of workloads and so are perfect candidates for evaluating when
materialized views are identified.
"""


# ------------------------------
# Query definitions

# >> Single-pass monotonic summary stats
class BaseSummaryStats:
    """
    A set of methods that define a query to efficiently calculate average and variance of
    gene expression in a single pass.
    """

    @classmethod
    def Accumulate(cls, data_table):
        squared_expr = (
              data_table['expr_val'].cast('int64')
            * data_table['expr_val'].cast('int64')
        )

        return (
            data_table.group_by(data_table.feature_name)
                      .aggregate(
                            cell_count=data_table.count()
                           ,expr_total=(data_table['expr_val'].cast('int64').sum())
                           ,expr_sumsq=squared_expr.sum()
                       )
        )

    @classmethod
    def Combine(cls, left_table, right_table):
        return (
            left_table.join(right_table, left_table.feature_name == right_table.feature_name)
                      .select(
                            left_table['feature_name'].name('feature_name')
                           ,(left_table['cell_count'] + right_table['cell_count']).name('cell_count')
                           ,(left_table['expr_total'] + right_table['expr_total']).name('expr_total')
                           ,(left_table['expr_sumsq'] + right_table['expr_sumsq']).name('expr_sumsq')
                       )
        )

    @classmethod
    def Complete(cls, data_table):
        expr_avg = data_table.expr_total / data_table.cell_count
        expr_var = (data_table.expr_sumsq / data_table.cell_count) - (expr_avg * expr_avg)
    
        return data_table.select(
             data_table.feature_name
            ,expr_avg.name('expr_avg')
            ,expr_var.name('expr_var')
        )
