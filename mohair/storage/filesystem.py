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
Code for providing a filesystem interface.
"""


# ------------------------------
# Dependencies

import sys
import pathlib

# modules
from pyarrow import ipc

# basic types
from pyarrow.flight import FlightDescriptor, FlightEndpoint

# higher-level types
from pyarrow.flight import FlightInfo


# ------------------------------
# Functions




# ------------------------------
# Classes

# TODO: could use a much better interface, but this will work for now.
class FileHandle:
    """
    TODO: figure out wtf this interface should be
    """

    @classmethod
    def FromFilePath(cls, file_path):
        return cls(file_path)

    def __init__(self, file_path, **kwargs):
        super().__init__(**kwargs)

        self._fpath = pathlib.Path(file_path)

    def ScanFile(self):
        sys.exit('Cannot call `ScanFile()` on a `FileHandle` instance')


class ArrowFileHandle(FileHandle):
    """
    A specialization of FileHandle for Arrow files (IPC file format or feather).
    """

    def __init__(self, file_path, **kwargs):
        super().__init__(file_path, **kwargs)

    def ScanFile(self):
        with self._fpath.open('rb') as fhandle:
            pass
