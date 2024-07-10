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
Skytether-specific relation types used by mohair. These are meant to bridge pure
relational structures to the application and storage data models of a cooperative
decomposition system.
"""


# ------------------------------
# Dependencies

# >> Standard modules
import sys

# >> Third-party modules
import pyarrow


# >> Standard classes
from dataclasses import dataclass, field


# ------------------------------
# Functions

# >> Write or Read skytether metadata

def EncodeMetaKey(key_name: str) -> bytes:
    """ Convenience function to encode a metadata key as utf-8. """
    return key_name.encode('utf-8')

#   |> Encoders
def EncodeSliceWidth(slice_width: int)  -> bytes:
    """ Convenience function to encode slice width in a `size_t` size. """
    return slice_width.to_bytes(8)

def EncodeSliceCount(slice_count: int)  -> bytes:
    """ Convenience function to encode slice count in a `size_t` size. """
    return slice_count.to_bytes(8)

def EncodeStripeSize(stripe_size: int) -> bytes:
    """ Convenience function to encode partition count in a `uint8_t` size. """
    return stripe_size.to_bytes(1)

#   |> Decoders
def DecodeSliceWidth(encoded_slicewidth: bytes) -> int:
    """ Convenience function to decode slice width (unsigned) from a `size_t` size. """
    return int.from_bytes(encoded_slicewidth)

def DecodeSliceCount(encoded_slicecount: bytes)  -> int:
    """
    Convenience function to decode slice count (unsigned) from a `size_t` size.
    """

    return int.from_bytes(encoded_slicecount)

def DecodeStripeSize(encoded_stripesize: bytes) -> int:
    """
    Convenience function to decode partition count (unsigned) from a `uint8_t` size.
    """

    return int.from_bytes(encoded_stripesize)


# >> Utility functions

def DefaultPartitionMetadata() -> dict[bytes, bytes]:
    return dict([
        (EncodeMetaKey(metakey), fn_encode(0))
        for metakey, fn_encode in [
             ('slice_width', EncodeSliceWidth)
            ,('slice_count', EncodeSliceCount)
            ,('stripe_size', EncodeStripeSize)
        ]
    ])

def BufferForRecordBatch(data_batch: pyarrow.RecordBatch) -> pyarrow.Buffer:
    """ Serializes an Arrow RecordBatch, :data_batch:, using IPC format. """

    ipc_buffer = pyarrow.BufferOutputStream()

    batch_writer = pyarrow.RecordBatchStreamWriter(ipc_buffer, data_batch.schema)
    batch_writer.write_batch(data_batch)
    batch_writer.close()

    return ipc_buffer.getvalue()


def SizesForTableBatches(data_table: pyarrow.Table, batch_size: int, count_samples=7):
    """
    Returns a generator (single-pass list) of the sizes (when serialized) of record
    batches of an Arrow Table, :data_table:, using the IPC stream format.
    """

    table_batches = data_table.to_batches(max_chunksize=batch_size)

    # how many samples we serialize
    if len(table_batches) < count_samples:
        count_samples = len(table_batches)

    return [
        BufferForRecordBatch(table_batches[batch_ndx]).size
        for batch_ndx in range(count_samples)
    ]


# shared_ptr<Table> table_data, int64_t target_bytesize, int64_t row_batchsize
def RowCountForByteSize(data_table, target_byte_size, row_batch_size=20480, max_itr=7):
    """
    Returns an estimated row count (chunk size) for batches of :data_table: that will fit
    within the target byte size :target_byte_size:.

    Using a maximum of :max_itr: attempts, the :data_table: is batched using
    :row_batch_size: and serialized to IPC. The ratio of the target byte size to the
    actual IPC buffer size then used to adjust :row_batch_size: until a serialized batch
    is smaller than the target byte size. Then, to accommodate variation between batches,
    a sampling (:max_itr:) of serialized batches is used to further reduce the estimated
    row count.
    """

    # get sizes of batches with initial parameters
    batch_byte_sizes = SizesForTableBatches(
        data_table, row_batch_size, count_samples=max_itr
    )

    # If there's no data or we failed to get sizes, return 0
    if len(batch_byte_sizes) < 1: return 0

    # conservative row count to return
    min_rowcount = 0

    # padding to accommodate margin of error across batches
    padded_byte_size = target_byte_size - 500

    # row_batchsize defaults to 20480, but should converge quickly over a few iterations
    itr_ndx        = 0
    for itr_ndx in range(max_itr):
        # Adjust row_batch_size until we fit in the desired envelope
        min_bytesize = batch_byte_sizes[0]
        min_rowcount = (padded_byte_size * row_batch_size) / min_bytesize

        if min_bytesize <= padded_byte_size: break

        row_batch_size   = min_rowcount
        batch_byte_sizes = SizesForTableBatches(
            data_table, row_batch_size, count_samples=max_itr
        )

    # We're done if there was only 1 batch
    if len(batch_byte_sizes) == 1: return min_rowcount

    # Otherwise, return the smallest row count across sampling of batches
    for sample_ndx in range(1, len(batch_byte_sizes), 1):
        sample_bytesize = batch_byte_sizes[sample_ndx]
        sample_rowcount = (padded_byte_size * row_batch_size) / sample_bytesize

        if sample_rowcount < min_rowcount:
            min_rowcount = sample_rowcount

    return min_rowcount


# ------------------------------
# Classes

@dataclass
class SkyDomain:
    key: str = 'public'

    def PartitionFor(self, partition_key: str) -> 'SkyPartition':
        return SkyPartition(domain=self, meta=SkyPartitionMeta(key=partition_key))

@dataclass
class SkyPartitionMeta:
    slice_width : int                = 0
    slice_count : int                = 0
    stripe_size : int                = 0
    key         : str                = None
    schema      : pyarrow.Schema     = None
    schema_meta : dict[bytes, bytes] = None


    def __str__(self) -> str:
        str_val = ''

        str_val += f'Slice Width: {self.slice_width}\n'
        str_val += f'Slice Count: {self.slice_count}\n'
        str_val += f'Stripe Size: {self.stripe_size}\n'

        str_val += str(self.schema)

        return str_val

    def SetSchema(self, pschema: pyarrow.Schema) -> 'SkyPartitionMeta':
        self.schema      = pschema
        self.schema_meta = pschema.metadata or DefaultPartitionMetadata()

        encoded_slicewidth = self.schema_meta[EncodeMetaKey('slice_width')]
        self.slice_width   = DecodeSliceWidth(encoded_slicewidth)

        encoded_slicecount = self.schema_meta[EncodeMetaKey('slice_count')]
        self.slice_count   = DecodeSliceCount(encoded_slicecount)

        encoded_stripesize = self.schema_meta[EncodeMetaKey('stripe_size')]
        self.stripe_size   = DecodeStripeSize(encoded_stripesize)

    def WithMetadata( self
                     ,slice_width=0
                     ,slice_count=0
                     ,stripe_size=1) -> 'SkyPartitionMeta':
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

        return self.schema

    def SetSliceWidth(self, new_slicewidth):
        metakey_slicewidth = EncodeMetaKey('slice_width')
        self.schema_meta[metakey_slicewidth] = EncodeSliceCount(new_slicewidth)

        self.slice_width = new_slicewidth
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetSliceCount(self, new_slicecount):
        metakey_slicecount = EncodeMetaKey('slice_count')
        self.schema_meta[metakey_slicecount] = EncodeSliceCount(new_slicecount)

        self.slice_count = new_slicecount
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

    def SetStripeSize(self, new_stripesize):
        metakey_stripesize = EncodeMetaKey('stripe_size')
        self.schema_meta[metakey_stripesize] = EncodeSliceCount(new_stripesize)

        self.stripe_size = new_stripesize
        self.schema = self.schema.with_metadata(self.schema_meta)

        return self

@dataclass
class SkyPartitionSlice:
    slice_index: int           = 0
    key        : str           = None
    data       : pyarrow.Table = None

    def num_rows(self)    -> int: return self.data.num_rows
    def num_columns(self) -> int: return self.data.num_columns

    def AsBatch(self) -> pyarrow.RecordBatch:
        return self.data.to_batches()[0]

@dataclass
class SkyPartition:
    domain: SkyDomain               = None
    meta  : SkyPartitionMeta        = None
    slices: list[SkyPartitionSlice] = field(default_factory=list)

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

    def SetSchema(self, pschema: pyarrow.Schema) -> 'SkyPartition':
        """ Updates the schema of this partition. """

        self.meta.SetSchema(pschema)

        return self

    def SetData(self, data_table: pyarrow.Table) -> 'SkyPartition':
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
                    ,data=pyarrow.Table.from_batches([pslice])
                )
            )

        return self

    def SetData(self, slice_ndx: int, slice_data: pyarrow.Table) -> 'SkyPartition':
        """
        Sets the data for this partition to the given `pyarrow.Table` (:data_table:).
        """

        # Each slice is treated as a table for simplicity
        self.slices.insert(slice_ndx, SkyPartitionSlice(slice_ndx, self.name(), slice_data))

        return self

