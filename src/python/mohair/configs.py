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
Default values for configurable settings.
"""


# ------------------------------
# Dependencies

from pathlib import Path


# ------------------------------
# Module Variables

# >> Bootstrap variables

#   |> a list of implemented database service backends


# >> Default variables

#   |> defaults for any service
GRPC_PORT = 9999

#   |> defaults for database backends
DB_FPATH   = Path('resources') / 'data'
DB_ENGINES = ['duckdb']
