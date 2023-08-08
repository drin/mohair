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
The execution portion of query processing for decomposable queries.
"""


# ------------------------------
# Dependencies

# >> Standard libs
import logging

from pathlib import Path

# >> Third-party
import pyarrow

#   |> modules
from pyarrow import substrait

# >> Internal

#   |> Logging
from mohair import CreateMohairLogger

#   |> classes
from mohair.query.substrait import SubstraitPlan


# ------------------------------
# Module variables

# >> Logging
logger = CreateMohairLogger(__name__)

SAMPLE_FPATH  = Path('resources') / 'examples' / 'sample-data.tsv'
SAMPLE_SCHEMA = pyarrow.schema([
     pyarrow.field('gene_id'   , pyarrow.string())
    ,pyarrow.field('cell_id'   , pyarrow.string())
    ,pyarrow.field('expression', pyarrow.float32())
])


# ------------------------------
# Classes


# ------------------------------
# Functions

def TableFromTSV(data_fpath: Path = SAMPLE_FPATH) -> pyarrow.Table:
    """
    Convenience function that creates an arrow table from :data_fpath: using hard-coded
    assumptions.
    """

    # read data from the given file; assume 3 columns (see `SAMPLE_SCHEMA`)
    with open(data_fpath) as data_handle:
        col_names   = next(data_handle).split(' ')
        data_by_col = [ [] for _ in col_names ]

        for line in data_handle:
            fields = line.strip().split(' ')

            data_by_col[0].append(fields[0])
            data_by_col[1].append(fields[1])
            data_by_col[2].append(float(fields[2]))

    # construct the table and return it
    return pyarrow.Table.from_arrays(
         [
              pyarrow.array(data_by_col[0], type=pyarrow.string())
             ,pyarrow.array(data_by_col[1], type=pyarrow.string())
             ,pyarrow.array(data_by_col[2], type=pyarrow.float32())
         ]
        ,schema=SAMPLE_SCHEMA
    )

def MohairTableProvider(table_names, expected_schema=None):
    for tname in table_names:
        logger.debug(f'Table requested: [{tname}]')

        if tname == 'expr': return TableFromTSV()

    return

def ExecuteSubstrait(substrait_plan: SubstraitPlan) -> pyarrow.Table:
    logger.debug('Executing substrait...')
    result_reader = substrait.run_query(
         substrait_plan.msg
        ,table_provider=MohairTableProvider
    )

    logger.debug('Query plan executed')
    return result_reader.read_all()
