import pyarrow

from pyarrow import flight, parquet


class SmartFileService(flight.FlightServerBase):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _make_flight_info(self, dataset):
        pass

    def list_flights(self, context, criteria):
        pass

    def get_flight_info(self, context, descriptor):
        pass

    def do_exchange(self, context, descriptor, reader, writer):
        pass

    def do_put(self, context, descriptor, reader, writer):
        pass

    def do_get(self, context, ticket):
        pass
