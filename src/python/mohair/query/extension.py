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

from typing      import Any, Annotated, TypeAlias, TypeVar
from dataclasses import dataclass

# >> Arrow
import pyarrow
from pyarrow import types

# >> Ibis
from ibis.common.patterns           import InstanceOf
from ibis.expr.types                import Table
from ibis.expr.operations.relations import UnboundTable

# >> Ibis-substrait
# NOTE: stalg is short for "substrait algebra";
#       stt   is short for "substrait types"
from ibis_substrait.compiler.translate import stalg, stt
from ibis_substrait.compiler.translate import translate

from ibis_substrait.compiler.core import SubstraitCompiler

# >> Internal
from mohair.query.types import SkyPartition, SkyDomain, SkyCatalog

from skyproto.substrait.type_pb2 import (NamedStruct, Type as SubstraitType)

from skyproto.mohair.algebra_pb2 import ( ExecutionStats
                                         ,SkyRel
                                         ,SkyPartitionRel
                                         ,SkySliceRel)


# ------------------------------
# Module Variables

# >> Forward references (Type aliases)
# type SkyPartition  = 'SkyPartition'
type SkyTable      = 'SkyTable'
type SkySliceTable = 'SkySliceTable'


from ibis_substrait.compiler.mapping import IBIS_SUBSTRAIT_TYPE_MAPPING
IBIS_SUBSTRAIT_TYPE_MAPPING['UInt16'] = 'u16'


# >> Substrait types
NullAttr    = SubstraitType.Nullability.NULLABILITY_NULLABLE
NonNullAttr = SubstraitType.Nullability.NULLABILITY_REQUIRED


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
    def FromPartition(cls, src_partition: SkyPartition) -> SkyTable:
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

    sky_partition: SkyPartition

    @classmethod
    def ForPartition(cls, src_partition: SkyPartition) -> SkySliceTable:
        return cls(
             name=src_partition.Name()
            ,schema=src_partition.schema
            ,sky_partition=src_partition
        )


# ------------------------------
# Functions

def SNullability(nullable: bool=False):
    return NullAttr if nullable else NonNullAttr

def STypeI8(nullable: bool=False) -> SubstraitType.I8:
    return SubstraitType.I8(nullability=SNullability(nullable))

def STypeI16(nullable: bool=False) -> SubstraitType.I16:
    return SubstraitType.I16(nullability=SNullability(nullable))

def STypeI32(nullable: bool=False) -> SubstraitType.I32:
    return SubstraitType.I32(nullability=SNullability(nullable))

def STypeI64(nullable: bool=False) -> SubstraitType.I64:
    return SubstraitType.I64(nullability=SNullability(nullable))

def STypeInt(width: int, nullable: bool=False) -> SubstraitType:
    if   width ==  8: return SubstraitType(i8=STypeI8(nullable))
    elif width == 16: return SubstraitType(i16=STypeI16(nullable))
    elif width == 32: return SubstraitType(i32=STypeI32(nullable))
    elif width == 64: return SubstraitType(i64=STypeI64(nullable))

    return None

def STypeFP32(nullable: bool=False) -> SubstraitType.FP32:
    return SubstraitType.FP32(nullability=SNullability(nullable))

def STypeFP64(nullable: bool=False) -> SubstraitType.FP64:
    return SubstraitType.FP64(nullability=SNullability(nullable))

def STypeFloat(width: int, nullable: bool=False) -> SubstraitType:
    if   width == 32: return SubstraitType(fp32=STypeFP32(nullable))
    elif width == 64: return SubstraitType(fp64=STypeFP64(nullable))

    return None

def STypeStr(nullable: bool=False) -> SubstraitType:
    return SubstraitType(
        string=SubstraitType.String(nullability=SNullability(nullable))
    )


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
        ,schema=translate(expr.schema())
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

    skyslice_rel = SkySliceRel(
         slice_key=op.sky_partition.Name()
        ,domain=op.sky_partition.domain
        ,partition=op.sky_partition.partition
        ,schema=translate(op.sky_partition.schema)
    )

    substrait_rel.extension_leaf.detail.Pack(skyslice_rel)

    return substrait_rel

@translate.register(pyarrow.Schema)
def _translate_arrow_type( pyschema: pyarrow.Schema
                          ,expr    : Table | None = None
                          ,*args   : Any
                          ,compiler: SubstraitCompiler | None = None
                          ,**kwargs: Any) -> NamedStruct:
    """ A translation function for direct support of pyarrow schema. """

    return NamedStruct(
         names=pyschema.names
        ,struct=SubstraitType.Struct(
              types=list(map(translate, pyschema.types))
             ,nullability=NonNullAttr
         )
    )

@translate.register(pyarrow.DataType)
def _translate_arrow_type( dtype   : pyarrow.DataType
                          ,expr    : Table | None = None
                          ,*args   : Any
                          ,compiler: SubstraitCompiler | None = None
                          ,**kwargs: Any) -> SubstraitType:
    """ A translation function for pyarrow data types. """

    if   types.is_int8(dtype):                            return STypeInt(8)
    elif types.is_uint8(dtype)  or types.is_int16(dtype): return STypeInt(16)
    elif types.is_uint16(dtype) or types.is_int32(dtype): return STypeInt(32)
    elif types.is_uint32(dtype) or types.is_int64(dtype): return STypeInt(64)

    elif types.is_float16(dtype) or types.is_float32(dtype): return STypeFloat(32)
    elif types.is_float64(dtype):                            return STypeFloat(64)

    elif types.is_string(dtype): return STypeStr()

    sys.exit(f'Unable to translate arrow type: {dtype}')
