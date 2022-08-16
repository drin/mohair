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
The various services we use in this prototype. Overall, these services are very
lightweight representations of what practical services might do. These implementations
are used to sketch the interactions of services in a computational storage system and how
we might query data from them. We want to explore the use of Substrait and Flight, but we
want to imagine how services that can handle these complex interfaces might co-exist with
simple interfaces (e.g. an S3-like interface or a KV interface backed by parquet files on
a local filesystem).
"""

# ------------------------------
# Dependencies

import pathlib

# modules
from pyarrow import parquet

# basic types
from pyarrow.flight import FlightDescriptor, FlightEndpoint

# higher-level types
from pyarrow.flight import FlightServerBase, FlightInfo


# ------------------------------
# Classes

# >> Convenience classes for object construction
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



# >> Client to computational storage server

class ApplicationService(FlightServerBase):
    """
    A facade for a service running on a client that takes application requests and
    converts them to queries against a storage server. This service only knows that the
    server it is talking to can receive queries of this form.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _make_flight_info(self, dataset):
        pass

    def list_flights(self, context, criteria):
        pass

    # >> Flight Verbs
    def get_flight_info(self, context, descriptor):
        pass

    def do_exchange(self, context, descriptor, reader, writer):
        pass

    def do_put(self, context, descriptor, reader, writer):
        pass

    def do_get(self, context, ticket):
        pass



# >> Storage servers that store data on computational storage devices

class MetadataService(FlightServerBase):
    """
    A facade for a service that stores data management metadata. This would be analagous
    to an "MDS" (metadata server) in Ceph, which can be queried for data placement (which
    storage server holds which objects), storage cluster information, as well as other
    metadata used by the storage system or describing data stored in the storage system.
    """

    def __init__(self, **kwargs):
        pass

    def _make_flight_info(self, dataset):
        pass

    def list_flights(self, context, criteria):
        pass

    # >> Flight Verbs
    def get_flight_info(self, context, descriptor):
        # TODO: This facade should be able to point clients to the correct storage service
        pass


class ComputationalStorageService(flight.FlightServerBase):
    """
    A facade for a storage service that runs on a server. This storage service is aware
    that it utilizes computational storage devices for data persistence.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _make_flight_info(self, dataset):
        pass

    def list_flights(self, context, criteria):
        pass

    # >> Flight Verbs
    def get_flight_info(self, context, descriptor):
        pass

    def do_exchange(self, context, descriptor, reader, writer):
        pass

    def do_put(self, context, descriptor, reader, writer):
        pass

    def do_get(self, context, ticket):
        pass


# >> Computational storage device
class SmartFileService(FlightServerBase):
    """
    A facade for an intelligent shim over a simple filesystem. This service keeps track of
    "datasets" that are actively being accessed, where a dataset is logical and maps 1-1
    to a physically available file. This service should be run at a location that serves
    data, which we have designed to be a location that does not know about data at other
    locations. This is a Ceph-like approach that keeps storage servers independent.

    For this service, a "flight" is a "dataset" is a file. The file format used will
    initially be IPC stream format.
    """

    def __init__(self, root_dir='/tmp', **kwargs):
        super().__init__(**kwargs)
        self.root_dir = root_dir

        # a way to identify what datasets are in memory
        #   <file descriptor: FlightDescriptor> -> <file ticket: FlightTicket>
        self._open_files = {}

    # TODO: Need to figure out how to enumerate URI Paths from a substrait query.
    # Reference:
    #   - https://substrait.io/relations/logical_relations/#read-definition-types
    # 1st pass: a single file
    # 2nd pass: a folder representing a partitioned file
    # 3rd pass: a glob representing files with matching schemas, each potentially a
    #           part of a partitioned file.
    def handle_from_descriptor(self, descriptor):
        if file_descr.path is not None:
            return self.root_dir / file_descr.path

        elif file_descr.cmd is not None:
            # file_handle = (SubstraitQuery.FromCommand(file_descr.cmd).URIPath())
            pass

        return ''

    def read_schema(self, flight_handle):
        # schema = parquet.read_schema(dataset_path)
        pass

    def list_flights(self, context, criteria):
        """
        Lists "available" flights, meaning flights that are readable at this location. The
        returned `FlightInfo` has non-zero `total_records` and `total_bytes` if the
        `FlightDescriptor` is being actively accessed (some portion of it is in memory).
        """

        for file_descr, file_tkt in self._open_files.items():
            # get a handle depending on what the descriptor contains
            flight_handle = self.handle_from_descriptor(file_descr)

            # get the schema for the file. This should also contain metadata
            schema = self.read_schema(flight_handle)

            # this seems to be how we send `FlightInfo` instances?
            yield FileInfo.ForDataset(file_descr, file_tkt, schema)

    # >> Flight Verbs
    def get_flight_info(self, context, descriptor):
        # TODO: This facade should only know how to serve data it stores
        pass

    def do_exchange(self, context, descriptor, reader, writer):
        pass

    def do_put(self, context, descriptor, reader, writer):
        pass

    def do_get(self, context, ticket):
        pass

class SmartDiskService(flight.FlightServerBase):
    """A facade for a smart disk (e.g. kinetic)"""

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
