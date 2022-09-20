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
Data types for representing storage components and objects. These data types are separated
into a separate module to simplify the services module.
"""


# ------------------------------
# Dependencies

import pathlib

# modules
from pyarrow import ipc

# basic types
from pyarrow.flight import FlightDescriptor, FlightEndpoint

# higher-level types
from pyarrow.flight import FlightInfo


# ------------------------------
# Classes

# >> Specializations for FlightDescriptor
class FileDescriptor:

    def __init__(self, **kwargs):
        super().__init__(**kwargs)


# >> Specializations for FlightTicket
class FileTicket:
    """
    Should be a structure that can be serialized to a byte string. Then, when deserialized
    it can be inspected for a type (substrait plan or file path), and then a generator can
    be returned that yields a stream of batches from whatever this is.
    """

    @classmethod
    def FromPath(cls, file_path):
        pass

    @classmethod
    def FromSubstrait(cls, file_queryplan):
        """
        A factory function that constructs a ticket from a substrait query plan. This
        function should only be called from a MetadataService (ceph-like architecture) or
        a SmartFileService (disk array architecture).
        """

        pass


# >> Specializations for FlightInfo
class FileInfo:
    """
    A namespace for convenience functions for `FlightInfo` objects representing files.
    """

    @classmethod
    def MetadataFromSchema(cls, file_schema):
        """
        Convenience function to extract metadata from the given file's schema. This
        metadata is packed with the schema to reduce IO requests.
        """

        pass

    @classmethod
    def ForDataset(cls, file_descr, file_tkt, file_schema, file_locs=[]):
        """
        Factory function to create a `FlightInfo` from file information. By default,
        :file_loc: is None because a `SmartFileService` only knows about files at its
        location. However, a `MetadataService` will be able to provide a :file_loc:.
        """

        endpoints = [FlightEndpoint(file_tkt, file_locs)]
        row_count, byte_count = cls.MetadataFromSchema(file_schema)

        return FlightInfo(file_schema, file_descr, endpoints, row_count, byte_count)


# TODO: could use a much better interface, but this will work for now.
class FileHandle:

    # TODO: Need to figure out how to enumerate URI Paths from a substrait query.
    # Reference:
    #   - https://substrait.io/relations/logical_relations/#read-definition-types
    # 1st pass: a single file
    # 2nd pass: a substrait plan listing a single source file
    # 3rd pass: a folder representing a partitioned file
    # 4th pass: a substrait plan listing a partitioned file
    # 5th pass: a glob representing files with matching schemas, each potentially a
    #           part of a partitioned file.
    @classmethod
    def FromDescriptor(cls, file_rootdir, file_descr):
        """
        Constructs a "handle" given a `FlightDescriptor`. The given `FlightDescriptor` can
        either be a string or opaque bytes: if it is a string we want to assume it is a
        file-like path or key, if it is opaque bytes we want to assume it is a substrait
        plan of some form (TBD).

        Currently expects :file_rootdir: to be a `pathlib.Path` object and returns a
        derived `pathlib.Path` object.
        """

        # NOTE: this is for 1st, 3rd, and 5th passes
        if file_descr.path is not None:
            # TODO: path is a list, so figure that out
            # I suspect it makes most sense for it to be a set of sources to pull batches
            # from or write batches to. The sender should include in the schema which file
            # to write to. Or, maybe it can be used to consolidate "small files"? Probably
            # too tricky...
            return file_rootdir / file_descr.path[0].decode('utf-8')

        # TODO: implement for 2nd and 4th passes
        elif file_descr.cmd is not None:
            # file_handle = (SubstraitQuery.FromCommand(file_descr.cmd).URIPath())
            pass

        return ''

    @classmethod
    def FromTicket(cls, file_rootdir, file_ticket):
        """
        Constructs a "handle" given a `FlightTicket`. The given `FlightTicket` is an
        "opaque" byte blob. But, in order for this to be a useful utility it implements
        the byte blob format. So, the byte blob is opaque to any file service using this
        `FileHandle`.
        """

        # TODO: ticket can be a general structure that says what it's type is. Then, based
        # on that type, it can decode into a query or it can decode into a path. The
        # appropriate decoding can be processed and the result should be something that
        # yields a data stream.
        pass
