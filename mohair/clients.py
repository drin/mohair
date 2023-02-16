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
Interface and implementations of client-side communications with a FlightServer.
"""


# ------------------------------
# Dependencies

# >> Third-party

#   |> Arrow Flight
from pyarrow import flight


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
        cls(flight.connect(service_location))

    def __init__(self, flight_conn, **kwargs):
        super().__init__(**kwargs)

        self.__flightconn = flight_conn

    def GetFlights(self):
        for flight in self.__flightconn.list_flights():
            print(flight)