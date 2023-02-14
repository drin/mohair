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
Database interface to storage.

For now, this uses duckdb as an anchor point for convenience. This interface will
generalize once we add support for other engines (such as sqlite or something).

Additionally, this is being prototyped in this repository but should eventually be moved
to skytether.
"""


# ------------------------------
# Dependencies

# >> Standard libs
import sys
import pathlib

# >> Third-party libs
import duckdb

# >> Arrow
#   |> Core types
from pyarrow import Table


# ------------------------------
# Classes

class DuckDBMS:

    @classmethod
    def Exists(cls, db_filepath):
        """ Checks if the database at :db_filepath: exists. """
        return Path(db_filepath).is_file()

    @classmethod
    def InFile(cls, db_filepath, mutable=True):
        """ Initialize a DuckDBMS instance as a new file-backed DuckDB database. """
        return cls(duckdb.connect(database=db_filepath, read_only=(not mutable)))

    @classmethod
    def InMemory(cls, arrow_src=None):
        """
        Initialize a DuckDBMS instance as a new in-memory DuckDB database.

        If :arrow_src: is specified, all queries are directed to it instead of an in-mem
        database. It is assumed that :data_src: is a valid Arrow object, see:
            https://duckdb.org/docs/guides/python/sql_on_arrow
        """

        return cls(duckdb.connect(database=':memory:'), data_src=arrow_src)

    def __init__(self, db_conn, data_src=None, **kwargs):
        """
        Initialize the DuckDB database given :db_conn:. It is expected that this is only
        called by a builder classmethod (such as `InMemory` or `InFile`):
            db = DuckDBMS.InMemory()

        If :data_src: is specified, all queries are directed to it instead of an in-mem
        database. It is assumed that :data_src: is a valid Arrow object, see:
            https://duckdb.org/docs/guides/python/sql_on_arrow
        """

        super().__init__(**kwargs)

        self.__data_src   = data_src
        self.__dbconn     = db_conn
        self.__table_list = []

    def ShowTables(self, use_cache=True) -> bool:
        """
        Get a list of all tables in the database. If :use_cache: is True (default), then
        do not query the database if we already have a list of all tables.
        """

        if not use_cache or not self.__table_list:
            self.__table_list = self.__dbconn.execute('SHOW TABLES').fetchall()

        return self.__table_list

    def TableExists(self, name: str, use_cache=True) -> bool:
        """
        Gets the table list using `ShowTables`. If :use_cache: is True (default), then
        `ShowTables` will not query the database if it already has a list of all tables.

        :returns: True if the table, :name:, is in the list of tables.
        """

        return name in self.ShowTables(use_cache)

    def CreateTable(self, name: str, arrow_table: Table, replace: False):
        """
        A convenience method for creating a database table. If this db instance was
        created from an Arrow object, this method will fail (return None).
        """

        # TODO: figure out a better way to propagate failure for this scenario.
        if self.__data_src is not None: return

        replace_clause = '' if not replace else 'OR REPLACE'
        self.__dbconn.execute(f'''
            CREATE TABLE {replace_clause} {name}
                      AS SELECT *
                           FROM arrow_table
        ''')

    def InsertData(self, name: str, arrow_table: Table):
        """ Convenience method for inserting many tuples into a table.  """

        self.__dbconn.execute(f'''
            INSERT INTO {name}
                 SELECT *
                   FROM arrow_table
        ''')

    def ScanData(self, name: str, limit=20):
        self.__dbconn.execute(f'''
            SELECT *
            FROM   {name}
            LIMIT  {limit}
        ''')

        return self.__dbconn.fetchall()

    def QueryData(self, query_str):
        self.__dbconn.execute(query_str)
        return self.__dbconn.fetchall()
