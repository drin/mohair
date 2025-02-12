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
import pyarrow

from pyarrow import types
from pyarrow import Schema, Table, RecordBatch

# >> Ibis-substrait (for translating a pyarrow.Schema to a substrait NamedStruct
from ibis.expr.schema                  import Schema as IbisSchema
from ibis_substrait.compiler.translate import translate

# >> Internal
from mohair import CreateMohairLogger
# from skyproto.mohair.algebra_pb2 import SkyRel, ExecutionStats

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
type MohairOp     = 'MohairOp'
type MohairPlan   = 'MohairPlan'
type SkyDomain    = 'SkyDomain'
type SkyPartition = 'SkyPartition'

type PartitionMap = dict[str, SkyPartition]


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
    op_inputs : tuple[MohairOp, ...]
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
    breaker_leaves: list[MohairPlan] = field(default_factory=list)
    breaker_list  : list[MohairPlan] = field(default_factory=list)

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


# ------------------------------
# Support functions for Mohair types

@dataclass
class SkyPartition:
    """ A data class for a skytether partition. """

    domain     : str
    partition  : str

    schema_meta: dict[bytes, bytes] = None
    schema     : Schema             = None

    slice_width: int = 0
    slice_count: int = 0
    stripe_size: int = 0

    @classmethod
    def InDomainWithKey(cls, dkey: str, pkey: str):
        return cls(dkey, pkey)

    def __hash__(self) -> int:
        return hash(self.Name())

    def __str__(self) -> str:
        return (
            f'Slice Width: {self.slice_width}\n'
            f'Slice Count: {self.slice_count}\n'
            f'Stripe Size: {self.stripe_size}\n'
            f'{self.schema}'
        )

    def Name(self):
        return f'{self.domain}/{self.partition}'

    def WithMetadata(self, new_meta: dict[bytes, bytes]) -> SkyPartition:
        """ Convenience method to set metadata and update schema. """

        # replace metadata
        self.schema_meta = new_meta

        # persist metadata to schema
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetSchema(self, pschema: Schema) -> SkyPartition:
        self.schema      = pschema
        self.schema_meta = pschema.metadata or DefaultPartitionMetadata()

        return self

    def SetSkytetherMeta(self, sl_width=0, sl_count=0, st_size=1) -> SkyPartition:
        """ Convenience method to set metadata and update schema.  """

        # Cache metadata
        self.slice_width = sl_width
        self.slice_count = sl_count
        self.stripe_size = st_size

        # replace metadata
        self.schema_meta = {
             EncodeMetaKey('slice_width'): EncodeSliceWidth(sl_width)
            ,EncodeMetaKey('slice_count'): EncodeSliceCount(sl_count)
            ,EncodeMetaKey('stripe_size'): EncodeStripeSize(st_size)
        }

        # return schema with new metadata
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetSliceWidth(self, new_slicewidth: int) -> SkyPartition:
        metakey_slicewidth = EncodeMetaKey('slice_width')
        metaval_slicewidth = EncodeSliceWidth(new_slicewidth)

        self.slice_width = new_slicewidth
        self.schema_meta[metakey_slicewidth] = metaval_slicewidth
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetSliceCount(self, new_slicecount: int) -> SkyPartition:
        metakey_slicecount = EncodeMetaKey('slice_count')
        metaval_slicecount = EncodeSliceCount(new_slicecount)

        self.slice_count = new_slicecount
        self.schema_meta[metakey_slicecount] = metaval_slicecount
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetStripeSize(self, new_stripesize: int) -> SkyPartition:
        metakey_stripesize = EncodeMetaKey('stripe_size')
        metaval_stripesize = EncodeStripeSize(new_stripesize)

        self.stripe_size = new_stripesize
        self.schema_meta[metakey_stripesize] = metaval_stripesize
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

@dataclass
class SkyDomain:
    """ A convenience class for managing a domain. """

    key       : str          = None
    partitions: PartitionMap = None

    @classmethod
    def WithKey(cls, domain_key: str, data: dict[str, SkyPartition]={}) -> SkyDomain:
        return cls(domain_key, data)

    def GetPartition(self, partition_key: str) -> SkyPartition:
        return self.partitions.get(partition_key)

    def RegisterPartition(self, pkey: str) -> SkyPartition:
        if not pkey in self.partitions:
            self.partitions[pkey] = SkyPartition.InDomainWithKey(self.key, pkey)

        return self.partitions[pkey]


@dataclass
class SkyCatalog:
    """ A convenience class for managing a catalog of domains. """

    partitions   : dict[str, PartitionMap] = field(default_factory=dict)
    active_domain: SkyDomain               = None

    @classmethod
    def WithDomain(cls, domain_key: str) -> 'SkyCatalog':
        new_catalog = cls()
        new_catalog.RegisterDomain(domain_key, set_active=True)

        return new_catalog

    def GetDomains(self) -> list[SkyDomain]:
        return [
            SkyDomain.WithKey(dkey, self.partitions[dkey]) 
            for dkey in self.partitions.keys()
        ]

    def GetDomain(self, domain_key: str=None) -> SkyDomain:
        return SkyDomain.WithKey(domain_key, self.partitions[domain_key])

    def GetPartitions(self, domain_key: str=None) -> PartitionMap:
        if domain_key is None: return self.active_domain.partitions

        return self.partitions.get(domain_key)

    def GetPartition(self, partition_key: str, domain_key: str=None) -> SkyPartition:
        if domain_key is None: return self.active_domain.GetPartition(partition_key)

        return self.partitions.get(domain_key, {}).get(partition_key)

    def SetActiveDomain(self, domain_key: str) -> SkyDomain:
        self.active_domain = self.GetDomain(domain_key)

        return self.active_domain

    def RegisterDomain(self, domain_key: str, set_active: bool=False) -> SkyDomain:
        # Add new domain
        if not domain_key in self.partitions:
            self.partitions[domain_key] = {}

        if set_active: return self.SetActiveDomain(domain_key)
        return self.GetDomain(domain_key)

    def RegisterPartition(self, pkey: str, dkey: str=None) -> SkyPartition:
        # Use active domain
        if dkey is None:
            return self.active_domain.RegisterPartition(pkey)

        # Domain mismatch
        if not dkey in self.partitions: return None

        # Register the partition
        domain_partitions = self.partitions[dkey]

        if pkey not in domain_partitions:
            domain_partitions[pkey] = SkyPartition.InDomainWithKey(dkey, pkey)

        return domain_partitions[pkey]

