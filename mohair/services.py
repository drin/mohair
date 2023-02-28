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
simple interfaces (e.g. an S3-like interface or a KV interface backed by IPC
files on a local filesystem).
"""


# ------------------------------
# Dependencies

# >> Standard libs
import pathlib

# >> Arrow
#   |> Core types
from pyarrow import ipc

#   |> Flight types
from pyarrow.flight import FlightDescriptor, FlightEndpoint
from pyarrow.flight import FlightServerBase, FlightInfo
from pyarrow.flight import Result

# >> Internal libs
#   |> Storage interfaces
from mohair.storage.dbms import DuckDBMS


# ------------------------------
# Classes

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


class DatabaseService(FlightServerBase):
    """
    A facade for a storage service that runs on a server and is backed by a DBMS. This
    storage service receives queries and executes them against the backing DBMS.
    """

    # TODO: consider suffixing db_fpath with skytether domain
    def __init__(self, service_location, db_fpath,  **kwargs):
        super().__init__(service_location, **kwargs)

        self.__service_loc = service_location

        if db_fpath and db_fpath.is_file():
            self.db         = DuckDBMS.InFile(db_fpath)
            self.__data_loc = db_fpath

        else:
            self.db         = DuckDBMS.InMemory()
            self.__data_loc = None

    def info(self):
        storage_type = 'in-memory'
        storage_loc  = ''

        if self.__data_loc:
            storage_type = 'file-backed'
            storage_loc  = f'({self.__data_loc})'

        return (
             'DatabaseService (DuckDB)\n'
            f'\tStorage type: {storage_type} {storage_loc}\n'
            f'\tService location: {self.__service_loc}'
        )

    def _make_flight_info(self, table_name):
        """
        Constructs a FlightInfo object by encoding the table name (str) to utf-8 bytes.
        Other location information (endpoints) is constructed from the service and data
        locations this DatabaseService was initialized with. Other metadata information is
        obtained from the database table (converted into an Arrow table).
        """

        print(f'Constructing FlightInfo for [{table_name}]')

        db_table = self.db.table(table_name).arrow()

        # flight objects
        db_descriptor = FlightDescriptor.for_path(table_name.encode('utf-8'))
        db_endpoints  = [
            FlightEndpoint(
                 self.__data_loc
                ,[self.__service_loc]
            )
        ]

        # TODO: possibly revisit descriptor; should be domain/partition
        return FlightInfo(
             db_table.schema
            ,db_descriptor
            ,db_endpoints
            ,db_table.num_rows
            ,db_table.nbytes
        )

    def list_flights(self, context, criteria):
        for table_name in self.db.ShowTables():
            yield self._make_flight_info(table_name)

    # >> Flight Verbs
    def get_flight_info(self, context, descriptor):
        """
        Constructs a FlightInfo object by decoding the descriptor path from utf-8 bytes to
        a string.
        """

        return self._make_flight_info(descriptor.path[0].decode('utf-8'))

    def do_exchange(self, context, descriptor, reader, writer):
        pass

    def do_put(self, context, descriptor, reader, writer):
        pass

    def do_get(self, context, ticket):
        """
        Handler for a `get` request. For mohair, a `ticket` is expected to be a substrait
        plan.
        """

        pass

    def do_action(self, context, action):
        """ Custom action handler.  """

        if action.type == 'query':
            action_result = self.action_query(action.body.to_pybytes())
        else:
            raise NotImplementedError

        return action_result

    def action_query(self, query_plan):
        """
        Handler for `query` action, which is called via `do_action('query', ...)`.

        This action expects a substrait plan as bytes, and should be able to parse the
        bytes using substrait protobuf wrappers.
        """

        print('Received query plan')
        yield Result('Received query plan'.encode('utf-8'))


class ComputationalStorageService(FlightServerBase):
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


# >> Computational file server
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

    def __init__(self, root_dirpath='/tmp', **kwargs):
        super().__init__(**kwargs)
        self.root_dir = pathlib.Path(root_dirpath)

        # a way to identify what datasets are in memory
        #   <file descriptor: FlightDescriptor> -> <file ticket: FlightTicket>
        self._open_files = {}

    def remove_dirtree(self, dirpath):
        if not dirpath.exists(): return

        for subpath in dirpath.iterdir():
            if subpath.is_dir(): self.remove_dirtree(subpath)
            else:                subpath.unlink(missing_ok=True)

        dirpath.rmdir()

    def init_storage(self):
        self.root_dir.mkdir(parents=True, exist_ok=True)

    def reset_storage(self):
        self.remove_dirtree(self.root_dir)
        self.root_dir.mkdir(parents=True, exist_ok=True)

    def read_schema(self, file_handle):
        """
        Reads the schema for a flight. The schema is used when returning `FlightInfo`
        structures. For now, it seems best to keep arbitrary metadata with the schema.
        This approach means that a schema and application metadata can be accessed in a
        consistent way with a single access.
        """

        # schema = ipc.read_schema(dataset_path)
        pass

    # >> Flight Verbs
    def list_flights(self, context, criteria):
        """
        Lists "available" flights, meaning flights that are readable at this location. The
        returned `FlightInfo` has non-zero `total_records` and `total_bytes` if the
        `FlightDescriptor` is being actively accessed (some portion of it is in memory).
        """

        # Foreach active descriptor: (1) get a handle, (2) get schema and metadata
        for file_descr, file_tkt in self._open_files.items():
            file_handle = FileHandle.FromDescriptor(self.root_dir, file_descr)
            schema      = self.read_schema(file_handle)

            yield FileInfo.ForDataset(file_descr, file_tkt, schema)

    def get_flight_info(self, context, flight_descr):
        # TODO: This facade should only know how to serve data it stores
        pass

    def do_get(self, context, ticket):
        """
        Flight verb that reads data described by :ticket: and returns the results to the
        client. In the simplest case, ticket may be a file path. In the most complex case,
        ticket may be a substrait plan.
        """

        flight_file = FileHandle.FromTicket(self.root_dir, ticket)
        with flight_file.open('wb') as file_handle:
            with ipc.new_file(file_handle, reader.schema) as flight_writer:
                for data_batch in reader:
                    flight_writer.write_batch(data_batch)

    def do_put(self, context, flight_descr, reader, writer):
        """
        Flight verb that writes data (from :reader:) to the handle described by
        :flight_descr:. Note that this verb overwrites the destination handle; to update
        the handle, use `do_exchange`.
        """

        # TODO: handle the multiplicity of files
        flight_files = FileHandle.FromDescriptor(self.root_dir, flight_descr)

        with flight_files[0].open('wb') as file_handle:
            with ipc.new_file(file_handle, reader.schema) as flight_writer:
                for data_batch in reader:
                    flight_writer.write_batch(data_batch)

    def do_exchange(self, context, flight_descr, reader, writer):
        """
        Flight verb that writes data (from :reader:) to the handle described by
        :flight_descr:. Note that this verb updates the destination handle; to simply
        overwrite the handle, use `do_put`.
        """

        flight_files = FileHandle.FromDescriptor(self.root_dir, flight_descr)

        with flight_files[0].open('wb') as file_handle:
            with ipc.new_file(file_handle, reader.schema) as flight_writer:
                for data_batch in reader:
                    flight_writer.write_batch(data_batch)


# >> Computational storage device
class SmartDiskService(FlightServerBase):
    """A facade for a smart disk (e.g. kinetic)"""

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
