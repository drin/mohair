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


# default_loglevel = logging.DEBUG
default_loglevel = logging.INFO


# ------------------------------
# Convenience functions

def AddConsoleLogHandler(mohair_logger, logger_level):
    '''
    Convenience function to centralize boiler plate for configuring a logger instance.
    '''

    console_loghandler = logging.StreamHandler()
    console_loghandler.setLevel(logger_level)
    console_loghandler.setFormatter(logging.Formatter(
        '%(name)s [%(levelname)s] |> %(message)s'
    ))

    mohair_logger.addHandler(console_loghandler)


def CreateMohairLogger(logger_name, logger_level=default_loglevel):
    '''
    Convenience function to centralize boiler plate for creating a logger instance.
    '''

    # Create instance
    logger = logging.getLogger(logger_name)

    # Configure instance
    logger.setLevel(logger_level)
    AddConsoleLogHandler(logger, logger_level)

    # Return instance
    return logger
