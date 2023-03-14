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
Convenience classes and functions for processing relational operators.
"""


# ------------------------------
# Dependencies

import logging


# ------------------------------
# Module Variables

__version__ = '0.1.0'


default_loglevel = logging.DEBUG
# default_loglevel = logging.INFO


# ------------------------------
# Convenience functions

def AddConsoleLogHandler(mohair_logger):
    console_loghandler = logging.StreamHandler()
    console_loghandler.setLevel(default_loglevel)
    console_loghandler.setFormatter(logging.Formatter(
        '%(name)s [%(levelname)s] |> %(message)s'
    ))

    mohair_logger.addHandler(console_loghandler)
