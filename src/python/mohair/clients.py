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
Interface and implementations of client-side communications with a FlightServer.
"""


# ------------------------------
# Dependencies

# >> Standard

import logging
import pdb

# >> Third-party
import pyarrow

#   |> Arrow Flight
from pyarrow import flight


# >> Internal

#   |> Logging
from mohair import CreateMohairLogger

#   |> Plan representation
# from mohair.query.operators import MohairPlan


# ------------------------------
# Module variables

# >> Logging
logger = CreateMohairLogger(__name__)


# ------------------------------
# Classes

# >> Client interface
class MohairClient:
    """
    An interface used by a client application that converts application intent into to
    queries against a mohair service. This service only knows that the server it is
    talking to can receive substrait plans.
    """

    @classmethod
    def ConnectTo(cls, service_location):
        logger.debug(f'Establishing connection to [{service_location}]')
        return cls(flight.connect(service_location))

    def __init__(self, flight_conn, **kwargs):
        super().__init__(**kwargs)

        self.__flightconn = flight_conn

    def GetFlights(self):
        """ Requests a list of flights from the remote mohair service. """

        for flight in self.__flightconn.list_flights():
            logger.debug(flight)

    def SendQueryPlan(self, substrait_plan):
        """ Sends a substrait plan (as bytes) to the remote mohair service. """

        # pdb.set_trace()
        log_results = self.__flightconn.do_action(flight.Action('query', substrait_plan))

        result_msg = next(log_results)
        while True:
            log_msg = result_msg.body

            try:
                result_msg = next(log_results)
                # logger.debug(log_msg.to_pybytes().decode('utf-8'))

            except StopIteration:
                break

        mohair_ticket = flight.Ticket(result_msg.body.to_pybytes())
        result_data   = self.__flightconn.do_get(mohair_ticket)
        # logger.debug(result_data.read_all())
        for data_result in result_data:
            logger.info(data_result.data.to_pandas())
