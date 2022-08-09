import pyarrow

from pyarrow import flight, parquet

class SmartFileService(flight.FlightServerBase):

    def __init__(self, **kwargs):
        pass

    # >> Flight Verbs
    def get_flight_info(self, context, descriptor):
        pass

    def do_exchange(self, context descriptor):
        pass
