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
# Overview
"""
Code that uses the ibis approach to compile substrait using mohair extensions.

Currently, the naive case is supported where we can compile to new message types such as
`SkyRel`--a logical read relation that is used by a Skytether storage service.

A more complex case is not yet supported, which would be to prioritize compiling to a new
or modified message type instead of a core substrait message type.
"""


# ------------------------------
# Dependencies

# >> Standard libs
from typing      import Any, Annotated, TypeVar
from dataclasses import dataclass


# >> Ibis
from ibis.common.patterns           import InstanceOf
from ibis.expr.types                import Table
from ibis.expr.operations.relations import UnboundTable

# >> Ibis-substrait
# NOTE: stalg is short for "substrait algebra"
from ibis_substrait.compiler.translate import stalg
from ibis_substrait.compiler.translate import translate

from ibis_substrait.compiler.core import SubstraitCompiler

# >> Internal
from mohair.query.operators import SkyPartition

from mohair.skytether.mohair.algebra_pb2 import SkyRel, ExecutionStats


# ------------------------------
# Classes

# SkyPartitionType = TypeVar('SkyPartitionType')
class SkyTable(UnboundTable):
    """
    A custom class that wraps a `SkyRel` in an ibis `Table` so that we can register a
    handler with `SubstraitCompiler.translate` for a `SkyRel` relation.

    We derive from `UnboundTable` since we call `unbind()` when generating substrait
    anyways.
    """

    # data_partition: Annotated[SkyPartitionType, InstanceOf(SkyPartition)]
    data_partition: SkyPartition

    @classmethod
    def FromPartition(cls, src_partition: SkyPartition) -> 'SkyTable':
        return cls(
             name=src_partition.name()
            ,schema=src_partition.schema()
            ,data_partition=src_partition
        )


# ------------------------------
# Functions

import pdb

@translate.register(SkyTable)
def _translate_mohair( op      : SkyTable
                      ,expr    : Table | None = None
                      ,*args   : Any
                      ,compiler: SubstraitCompiler | None = None
                      ,**kwargs: Any) -> stalg.Rel:
    """
    A custom translation function that is invoked when `op` is an instance of `SkyTable`.
    """

    substrait_rel = stalg.Rel(
        extension_leaf=stalg.ExtensionLeafRel(
             common=stalg.RelCommon(direct=stalg.RelCommon.Direct())
        )
    )

    #pdb.set_trace()

    # extension_leaf.detail is an Any message, so we use its Pack method on SkyRel
    sky_rel = SkyRel(
        domain=op.data_partition.domain.key
       ,partition=op.data_partition.meta.key
       ,slices=op.data_partition.slice_indices()
       ,execstats=op.data_partition.exec_stats()
    )

    substrait_rel.extension_leaf.detail.Pack(sky_rel)

    return substrait_rel

