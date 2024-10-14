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
Substrait and mohair types.

This includes Skytether-specific relation types used by mohair which are meant to bridge
pure relational structures to the application and storage data models of a cooperative
decomposition system.
"""


# ------------------------------
# Dependencies

# >> Standard libs
from dataclasses import dataclass, field
from typing import Any, TypeAlias

# >> Arrow
from pyarrow import Schema, Table, RecordBatch

# >> Internal
from mohair import CreateMohairLogger
from mohair.mohair.algebra_pb2 import SkyRel, ExecutionStats

# convenience functions for metadata management
from mohair.util import ( DefaultPartitionMetadata
                         ,EncodeMetaKey
                         ,EncodeSliceWidth, EncodeSliceCount, EncodeStripeSize
                         ,DecodeSliceWidth, DecodeSliceCount, DecodeStripeSize)

# support functions for physical design
from mohair.util import RowCountForByteSize


# ------------------------------
# Module Variables

# >> Logging
logger = CreateMohairLogger(__name__)

# >> Forward references (Type aliases)
OpTypeAlias       : TypeAlias = 'MohairOp'
PlanTypeAlias     : TypeAlias = 'MohairPlan'
PartitionTypeAlias: TypeAlias = 'SkyPartition'
MetaTypeAlias     : TypeAlias = 'SkyPartitionMeta'


# ------------------------------
# Classes

# >> Operator base classes
@dataclass
class MohairOp:
    """
    A single operator in a query plan that primarily maintains references to operators in
    the substrait plan.
    """

    plan_op   : Any
    op_inputs : tuple[OpTypeAlias, ...]
    table_name: str = ''

    def __str__(self):
        op_name = self.plan_op.DESCRIPTOR.name
        return f'{op_name}({self.table_name})'

@dataclass
class PipelineOp(MohairOp):
    """ A query operator that tuples can stream through. """

    def ViewStr(self): return f'← {self}'


@dataclass
class BreakerOp(MohairOp):
    """ A stateful query operator that tuples cannot naively stream through. """

    def ViewStr(self): return f'↤ {self}'


# >> Graph classes

@dataclass
class MohairPlan:
    """
    Query plan that propagates through a computational storage system. Also stores
    properties of the plan for later analysis.

    :plan_root:      The `MohairOp` that is the root of this plan.
    :pipeline_len:   The length of the longest pipeline.
    :breaker_height: The max count of pipeline breakers in the plan.
    :breaker_count:  The total count of breakers in the plan.
    :breaker_leaves: The "bottom-most" pipeline breakers in the query plan.
    """

    plan_root: MohairOp

    pipeline_len  : int = 0
    plan_width    : int = 0
    plan_height   : int = 0
    breaker_count : int = 0
    breaker_height: int = 0
    breaker_leaves: list[PlanTypeAlias] = field(default_factory=list)
    breaker_list  : list[PlanTypeAlias] = field(default_factory=list)

    def __hash__(self):
        plan_hash = hash(self.plan_root)
        logger.debug(f'Hash of MohairPlan: {plan_hash}')

        return plan_hash

    def IncrementPipelineLength(self) -> int:
        """
        Function to get an incremented pipeline length. If this plan is rooted with a
        pipeline-able operator, then :pipeline_len: is incremented, otherwise 1 is
        returned.
        """

        if issubclass(type(self.plan_root), BreakerOp): return 1
        return self.pipeline_len + 1

@dataclass
class DecomposedPlan:
    """ Portion of a query plan that is *not* delegated downstream. """

    query_plan     : MohairPlan
    anchor_root    : MohairPlan
    anchor_subplans: tuple[MohairOp, ...]


@dataclass
class LogicalExecPlan(MohairPlan):
    pass


# ------------------------------
# Skytether classes

@dataclass
class SkyDomain:
    """ A convenience class for managing domains. """

    key: str = 'public'

    def PartitionFor(self, partition_key: str) -> PartitionTypeAlias:
        return SkyPartition(domain=self, meta=SkyPartitionMeta(key=partition_key))

@dataclass
class SkyPartitionMeta:
    """ A data class for partition metadata. """

    slice_width: int                = 0
    slice_count: int                = 0
    stripe_size: int                = 0
    key        : str                = None
    schema_meta: dict[bytes, bytes] = None
    schema     : Schema             = None


    def __str__(self) -> str:
        str_val = ''

        str_val += f'Slice Width: {self.slice_width}\n'
        str_val += f'Slice Count: {self.slice_count}\n'
        str_val += f'Stripe Size: {self.stripe_size}\n'

        str_val += str(self.schema)

        return str_val

    def WithMetadata(self, new_meta: dict[bytes, bytes]) -> MetaTypeAlias:
        """ Convenience method to set metadata and update schema. """

        # replace metadata
        self.schema_meta = new_meta

        # persist metadata to schema
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetSchema(self, pschema: Schema) -> MetaTypeAlias:
        self.schema      = pschema
        self.schema_meta = pschema.metadata or DefaultPartitionMetadata()

    def SetSkytetherMeta(self, slice_width=0, slice_count=0, stripe_size=1) -> MetaTypeAlias:
        """ Convenience method to set metadata and update schema.  """

        # Cache metadata
        self.slice_width = slice_width
        self.slice_count = slice_count
        self.stripe_size = stripe_size

        # replace metadata
        self.schema_meta = {
             EncodeMetaKey('slice_width'): EncodeSliceWidth(slice_width)
            ,EncodeMetaKey('slice_count'): EncodeSliceCount(slice_count)
            ,EncodeMetaKey('stripe_size'): EncodeStripeSize(stripe_size)
        }

        # return schema with new metadata
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetSliceWidth(self, new_slicewidth: int) -> MetaTypeAlias:
        metakey_slicewidth = EncodeMetaKey('slice_width')
        metaval_slicewidth = EncodeSliceWidth(new_slicewidth)

        self.slice_width = new_slicewidth
        self.schema_meta[metakey_slicewidth] = metaval_slicewidth
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetSliceCount(self, new_slicecount: int) -> MetaTypeAlias:
        metakey_slicecount = EncodeMetaKey('slice_count')
        metaval_slicecount = EncodeSliceCount(new_slicecount)

        self.slice_count = new_slicecount
        self.schema_meta[metakey_slicecount] = metaval_slicecount
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetStripeSize(self, new_stripesize: int) -> MetaTypeAlias:
        metakey_stripesize = EncodeMetaKey('stripe_size')
        metaval_stripesize = EncodeStripeSize(new_stripesize)

        self.stripe_size = new_stripesize
        self.schema_meta[metakey_stripesize] = metaval_stripesize
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self


@dataclass
class SkyPartitionSlice:
    """ A data class that couples a slice's index, key name, and table data. """

    slice_index: int   = 0
    key        : str   = None
    data       : Table = None

    def num_rows(self)    -> int: return self.data.num_rows
    def num_columns(self) -> int: return self.data.num_columns

    def AsBatch(self) -> RecordBatch:
        return self.data.to_batches()[0]


@dataclass
class SkyPartition:
    """
    A data class that couples the parts of a partition (domain, metadata, slice).

    This is used to compile substrait using ibis (or some producer) or to extract
    information from a substrait operator (SkyRel).
    """

    domain: SkyDomain               = None
    meta  : SkyPartitionMeta        = None
    slices: list[SkyPartitionSlice] = field(default_factory=list)
    stats : ExecutionStats          = None

    @classmethod
    def FromOp(cls, query_op: SkyRel) -> PartitionTypeAlias:
        return cls(
             domain=SkyDomain(query_op.domain)
            ,meta=SkyPartitionMeta(key=query_op.partition)
            ,slices=[
                 SkyPartitionSlice(slice_index=slice_ndx)
                 for slice_ndx in query_op.slices
             ]
            ,stats=query_op.execstats
        )

    def __post_init__(self):
        if self.stats is None:
            self.stats = ExecutionStats(executed=False)

    def __hash__(self):
        return hash(self.domain.key) + hash(self.meta.key)

    def key(self):
        return self.meta.key

    def name(self, domain_delim='/'):
        return f'{self.domain.key}{domain_delim}{self.meta.key}'

    def schema(self):
        return self.meta.schema

    def slice_indices(self) -> list[int]:
        return [
            partition_slice.slice_index
            for partition_slice in self.slices
            if partition_slice is not None
        ]

    def exec_stats(self) -> ExecutionStats:
        return self.stats

    def SetSchema(self, pschema: Schema) -> 'SkyPartition':
        """ Updates the schema of this partition. """

        self.meta.SetSchema(pschema)

        return self

    def SetData(self, data_table: Table) -> PartitionTypeAlias:
        """
        Sets the data for this partition to the given `pyarrow.Table` (:data_table:).
        """

        # initialize (or clear) partition data
        self.slices = []

        # Some variables to find N rows to produce 1 MiB record batches
        max_slice_bytesize = 1024 * 1024
        slice_row_count    = RowCountForByteSize(data_table, max_slice_bytesize)

        if slice_row_count == 0:
            sys.exit(f'Failed to determine row count for partition {self.name()}')

        # Create a partition slices for each recordbatch
        for ndx, pslice in enumerate(data_table.to_batches(max_chunksize=slice_row_count)):

            # Each slice is treated as a table for simplicity
            self.slices.append(
                SkyPartitionSlice(
                     slice_index=ndx
                    ,key=self.name()
                    ,data=Table.from_batches([pslice])
                )
            )

        return self

    def SetData(self, slice_ndx: int, slice_data: Table) -> PartitionTypeAlias:
        """
        Sets the data for this slice to the given `pyarrow.Table` (:data_table:); each
        slice is treated as a table for simplicity.
        """

        # Overwrite the existing slice
        if slice_ndx < self.meta.slice_count:
            self.slices[slice_ndx] = SkyPartitionSlice(slice_ndx, self.name(), slice_data)

        # Otherwise, append to our list of slices
        else:
            self.slices.append(
                SkyPartitionSlice(self.meta.slice_count, self.name(), slice_data)
            )
            self.meta.slice_count += 1

        return self
