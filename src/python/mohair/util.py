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
import logging
import argparse

from mohair import CreateMohairLogger
from mohair import default_loglevel


# ------------------------------
# Module variables

# >> Logging
logger = CreateMohairLogger(__name__, logger_level=default_loglevel)


# ------------------------------
# utility classes
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

    def add_output_file_arg(self, required=False, help_str=''):
        self._arg_parser.add_argument(
             '--output-file'
            ,dest='output_file'
            ,type=str
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

    def parse_args(self):
        return self._arg_parser.parse_known_args()
