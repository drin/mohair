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
Convenience classes and functions expected to be reused across other modules.
"""


# ------------------------------
# Dependencies

import sys
import argparse
import pyarrow

from mohair import CreateMohairLogger


# ------------------------------
# Module variables

# >> Logging
logger = CreateMohairLogger(__name__)


# ------------------------------
# Utility functions

# >> Readers
def TableFromBinary(ipc_stream_buffer):
    """ Opens an IPC stream from :ipc_stream_buffer: and returns an Arrow Table. """
    stream_reader = pyarrow.ipc.open_stream(ipc_stream_buffer)

    return stream_reader.read_all()


def SchemaFromBinary(ipc_stream_buffer):
    """ Opens an IPC stream from :data_blob: and returns an Arrow Table. """
    stream_reader = pyarrow.ipc.open_stream(ipc_stream_buffer)

    return stream_reader.schema


# >> Writers
def BinaryFromRecordBatch(data_batch, stream_writer=None):
    """ Serializes an Arrow RecordBatch, :data_batch:, using IPC format. """

    use_tmp_buffer = stream_writer is None

    # If not given a writer, create a buffer and a writer that targets it
    if use_tmp_buffer:
        arrow_buffer  = pyarrow.BufferOutputStream()
        stream_writer = pyarrow.RecordBatchStreamWriter(arrow_buffer, data_batch.schema)

    # Actually serialize the batch to the buffer
    stream_writer.write_batch(data_batch)

    # If we created the writer, return the serialized buffer
    if use_tmp_buffer:
        stream_writer.close()
        return arrow_buffer.getvalue()

    # If given a writer, just return None (like a 'void' function)
    return None


def BinaryFromTable(data_table):
    """ Serializes an Arrow Table, :data_table:, using IPC format. """

    arrow_buffer  = pyarrow.BufferOutputStream()
    stream_writer = pyarrow.RecordBatchStreamWriter(arrow_buffer, data_table.schema)

    for record_batch in data_table.to_batches():
        stream_writer.write_batch(record_batch)

    stream_writer.close()

    return arrow_buffer.getvalue()


def BinaryFromSchema(data_schema):
    """ Serializes an Arrow Schema, :data_schema:, using IPC format. """

    arrow_buffer  = pyarrow.BufferOutputStream()
    stream_writer = pyarrow.RecordBatchStreamWriter(arrow_buffer, data_schema)
    stream_writer.close()

    return arrow_buffer.getvalue()


# >> Skytether metadata encoders and decoders

# Value Encoders

def EncodeSliceWidth(slice_width: int)  -> bytes:
    """ Convenience function to encode slice width in a `size_t` size. """
    return slice_width.to_bytes(8)

def EncodeSliceCount(slice_count: int)  -> bytes:
    """ Convenience function to encode slice count in a `size_t` size. """
    return slice_count.to_bytes(8)

def EncodeStripeSize(stripe_size: int) -> bytes:
    """ Convenience function to encode partition count in a `uint8_t` size. """
    return stripe_size.to_bytes(1)


# Value Decoders
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

# Convenience encoder/decoder functions
def EncodeMetaKey(key_name: str) -> bytes:
    """ Convenience function to encode a metadata key as utf-8. """
    return key_name.encode('utf-8')


# >> Skytether utility functions
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
# Utility classes
class ArgparseBuilder(object):
    """
    A wrapper that makes it easier to build a parser for CLI arguments.

    This is primarily for tools that aren't part of the main CLI that uses click.
    """

    @classmethod
    def with_description(cls, program_descr='Default program'):
        return cls(argparse.ArgumentParser(description=program_descr))

    def __init__(self, arg_parser, **kwargs):
        super(ArgparseBuilder, self).__init__(**kwargs)

        self._arg_parser = arg_parser

    def add_input_dir_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--input-dir'
            ,dest='input_dir'
            ,type=str
            ,required=required
            ,help=(help_str or
                   'Path to directory containing input files for this program to process')
        )

        return self

    def add_config_file_arg(self, required=False, help_str='', default=''):
        self._arg_parser.add_argument(
             '--config-file'
            ,dest='config_file'
            ,type=str
            ,required=required
            ,help=(help_str or 'Path to config file for this program to process')
        )

        return self

    def add_input_file_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--input-file'
            ,dest='input_file'
            ,type=str
            ,required=required
            ,help=(help_str or 'Path to file for this program to process')
        )

        return self

    def add_db_file_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--db-file'
            ,dest='db_file'
            ,type=str
            ,required=required
            ,help=(help_str or 'Path to duckdb database file.')
        )

        return self

    def add_input_metadata_file_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--input-metadata'
            ,dest='input_metadata_file'
            ,type=str
            ,required=required
            ,help=(help_str or 'Path to file containing metadata for this program to process')
        )

        return self

    def add_metadata_target_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--metadata-target'
            ,dest='metadata_target'
            ,type=str
            ,required=required
            ,help=(help_str or 'Type of data described by provided metadata: <cells | genes>')
        )

        return self

    def add_delimiter_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--field-delimiter'
            ,dest='field_delimiter'
            ,type=str
            ,required=required
            ,default='\t'
            ,help=(help_str or 'String to use as delimiter between fields of each record (line)')
        )

        return self

    def add_data_format_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--data-format'
            ,dest='data_format'
            ,type=str
            ,required=required
            ,help=(help_str or 'Serialization format for data')
        )

        return self

    def add_query_dialect_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--query-dialect'
            ,dest='query_dialect'
            ,type=str
            ,choices=['duckdb', 'ibis']
            ,default='duckdb'
            ,required=required
            ,help=(help_str or 'Dialect (language) of input query')
        )

        return self

    def add_output_file_arg(self, default_fpath=None, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--output-file'
            ,dest='output_file'
            ,type=str
            ,default=default_fpath
            ,required=required
            ,help=(help_str or 'Path to file for this program to produce')
        )

        return self

    def add_output_file_format_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--output-file-format'
            ,dest='output_file_format'
            ,type=str
            ,required=required
            ,help=(help_str or 'File format to use when serializing output data')
        )

        return self

    def add_output_dir_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--output-dir'
            ,dest='output_dir'
            ,type=str
            ,required=required
            ,help=(help_str or 'Path to directory for output files to be written')
        )

        return self

    def add_batch_args(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--batch-size'
            ,dest='batch_size'
            ,type=int
            ,required=required
            ,help=(help_str or 'Amount of entities to include in each batch (aka window size)')
        )

        self._arg_parser.add_argument(
             '--batch-offset'
            ,dest='batch_offset'
            ,type=int
            ,required=required
            ,help=(help_str or 'Amount of entities to shift the batch start (aka stride)')
        )

        return self

    def add_skyhook_query_bin_args(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--query-bin'
            ,dest='path_to_query_bin'
            ,type=str
            ,default='/mnt/sdb/skyhookdm-ceph/build/bin/run-query'
            ,required=required
            ,help=(help_str or "Path to skyhook's run-query binary")
        )

        return self

    def add_query_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--query-str'
            ,dest='query_str'
            ,type=str
            ,required=required
            ,help=(help_str or 'Structured query expression')
        )

        return self

    def add_skyhook_obj_slice_args(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--start-obj'
            ,dest='start_obj'
            ,type=str
            ,default='0'
            ,required=False
            ,help=(help_str or 'First partition to do object scan from')
        )

        self._arg_parser.add_argument(
             '--num-objs'
            ,dest='num_objs'
            ,type=str
            ,default='1000'
            ,required=required
            ,help=(help_str or 'Number of objects (including start) to scan')
        )

        return self

    def add_flatbuffer_flag_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--use-skyhook-wrapper'
            ,dest='flag_use_wrapper'
            ,action='store_true'
            ,required=required
            ,help=(help_str or "Whether to use skyhook's flatbuffer wrapper. <True | False>")
        )

        return self

    def add_has_header_flag_arg(self, required=False, help_str=''):
        default_help_msg = (
            "Specifies that cells and genes list files have header lines to skip. "
            "<True | False>"
        )

        self._arg_parser.add_argument(
             '--data-files-have-header'
            ,dest='flag_has_header'
            ,action='store_true'
            ,required=required
            ,help=(help_str or default_help_msg)
        )

        return self

    def add_prepend_header_flag_arg(self, required=False, help_str=''):
        default_help_msg = (
            "Specifies that a header for cells or genes annotation files should be inserted. "
            "<True | False>"
        )

        self._arg_parser.add_argument(
             '--prepend-metadata-header'
            ,dest='flag_add_header'
            ,action='store_true'
            ,required=required
            ,help=(help_str or default_help_msg)
        )

        return self

    def add_analysis_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--should-analyze'
            ,dest='should_analyze'
            ,action='store_true'
            ,required=required
            ,help=(help_str or 'Flag representing whether to do runtime analysis')
        )

        return self

    def add_ceph_pool_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--pool'
            ,dest='ceph_pool'
            ,type=str
            ,required=required
            ,help=(help_str or 'Name of Ceph pool to use.')
        )

        return self

    def add_skyhook_table_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--table'
            ,dest='skyhook_table'
            ,type=str
            ,required=required
            ,help=(help_str or 'Name of table to be used in SkyhookDM naming scheme')
        )

        return self

    def add_skytether_domain_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--domain'
            ,dest='domain_key'
            ,type=str
            ,required=required
            ,help=(help_str or 'Name of data domain to access data from.')
        )

        return self

    def add_skytether_partition_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--partitions'
            ,dest='partition_keys'
            ,type=str
            ,nargs='+'
            ,required=required
            ,help=(help_str or 'Name of data partitions to access data from (one or more).')
        )

        return self

    def parse_args(self):
        return self._arg_parser.parse_known_args()
