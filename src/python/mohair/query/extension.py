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
import sys

from typing      import Any, Annotated, TypeAlias
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
from mohair.query.types import SkyPartition

from skyproto.mohair.algebra_pb2 import ( ExecutionStats
                                         ,SkyRel
                                         ,SkyPartitionRel
                                         ,SkySliceRel)


# ------------------------------
# Module Variables

# >> Forward references (Type aliases)
SkyTableType    : TypeAlias = 'SkyTable'
SkyPartitionType: TypeAlias = 'SkyPartitionTable'
SkySliceType    : TypeAlias = 'SkySliceTable'


from ibis_substrait.compiler.mapping import IBIS_SUBSTRAIT_TYPE_MAPPING
IBIS_SUBSTRAIT_TYPE_MAPPING['UInt16'] = 'u16'



# ------------------------------
# Classes

class SkyTable(UnboundTable):
    """
    A custom class that wraps a `SkyRel` in an ibis `Table` so that we can register a
    handler with `SubstraitCompiler.translate` for a `SkyRel` relation.

    We derive from `UnboundTable` since we call `unbind()` when generating substrait
    anyways.
    """

    data_partition: SkyPartition

    @classmethod
    def FromPartition(cls, src_partition: SkyPartition) -> SkyTableType:
        return cls(
             name=src_partition.name()
            ,schema=src_partition.schema()
            ,data_partition=src_partition
        )

# >> TODO: build out this custom IR type for directly naming partitions
class SkyPartitionTable(SkyTable): pass

class SkySliceTable(UnboundTable):
    """
    Ibis op representing a directly named skytether data slice. This may be used for a
    skytether partition that is stored only using a data slice (no independent meta
    slice).
    """

    data_partition: SkyPartition
    slice_ndx     : int
    slice_key     : str

    @classmethod
    def ForPartition(cls, src_partition: SkyPartition) -> SkySliceType:
        key_name = f'{src_partition.domain.key}/{src_partition.meta.key}'

        return cls(
             name=src_partition.name()
            ,schema=src_partition.schema()
            ,data_partition=src_partition
            ,slice_ndx=0
            ,slice_key=key_name
        )

    @classmethod
    def ForSlice(cls, src_partition: SkyPartition, slice_ndx: int) -> SkySliceType:
        key_name = (
              src_partition.domain.key
            + f'/{src_partition.meta.key}'
            + f'-{slice_ndx}'
        )

        return cls(
             name=src_partition.name()
            ,schema=src_partition.schema()
            ,data_partition=src_partition
            ,slice_ndx=slice_ndx
            ,slice_key=key_name
        )


# ------------------------------
# Functions

# >> TODO: create a translation for this type
#     elif type(op) is SkyPartitionTable:
#         sky_rel = SkyPartitionRel(
#             domain=op.data_partition.domain.key
#            ,partition=op.data_partition.meta.key
#            ,slices=op.data_partition.slice_indices()
#            ,execstats=op.data_partition.exec_stats()
#         )

# >> Dispatch functions for custom translations
@translate.register(SkyTable)
def _translate_skyrel( op      : SkyTable
                      ,expr    : Table | None = None
                      ,*args   : Any
                      ,compiler: SubstraitCompiler | None = None
                      ,**kwargs: Any) -> stalg.Rel:
    """ A translation function for `SkyTable` operator. """

    substrait_rel = stalg.Rel(
        extension_leaf=stalg.ExtensionLeafRel(
             common=stalg.RelCommon(direct=stalg.RelCommon.Direct())
        )
    )

    # extension_leaf.detail is an Any message, so we use its Pack method on SkyRel
    sky_rel = SkyRel(
        domain=op.data_partition.domain.key
       ,partition=op.data_partition.meta.key
       ,execstats=op.data_partition.exec_stats()
    )

    substrait_rel.extension_leaf.detail.Pack(sky_rel)

    return substrait_rel

@translate.register(SkySliceTable)
def _translate_skyslice( op      : SkySliceTable
                        ,expr    : Table | None = None
                        ,*args   : Any
                        ,compiler: SubstraitCompiler | None = None
                        ,**kwargs: Any) -> stalg.Rel:
    """ A translation function for `SkySliceTable` operator. """

    substrait_rel = stalg.Rel(
        extension_leaf=stalg.ExtensionLeafRel(
             common=stalg.RelCommon(direct=stalg.RelCommon.Direct())
        )
    )

    slice_rel = SkySliceRel(
        slice_key=op.slice_key
       ,domain=op.data_partition.domain.key
       ,partition=op.data_partition.meta.key
       ,slice=op.slice_ndx
       ,execstats=op.data_partition.exec_stats()
    )

    substrait_rel.extension_leaf.detail.Pack(slice_rel)

    return substrait_rel
