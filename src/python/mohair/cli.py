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
# Dependencies

# >> Standard libs
import sys
import logging

from pathlib import Path

# >> Third-party
import click

# >> Internal libs

#   |> Logging
from mohair import CreateMohairLogger

#   |> Clients and Services
from mohair.clients import MohairClient
from mohair.services import DatabaseService

#   |> module variables
from mohair.configs import GRPC_PORT, DB_FPATH, DB_ENGINES


# ------------------------------
# Module variables

# >> Logging
logger = CreateMohairLogger(__name__)


# ------------------------------
# Classes (for CLI)

class ServiceContext:
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.host    = 'localhost'
        self.port    = GRPC_PORT
        self.service = None

    def listen_uri(self):
        if self.host == 'localhost':
            return f'grpc://0.0.0.0:{self.port}'

        return f'grpc://{self.host}:{self.port}'

    def connect_uri(self):
        return f'grpc://{self.host}:{self.port}'


# ------------------------------
# Functions (for CLI)

# >> Create a decorator from `ServiceContext`
cli_service_ctx = click.make_pass_decorator(ServiceContext)


# >> Groups of CLI commands

#   |> Top-level command group
@click.group()
@click.version_option('1.0')
@click.pass_context
def cli(ctx):
    """
    A CLI for interacting with mohair, a decomposable query processor.

    Some of what is provided in this CLI is for testing and example purposes, such as
    starting a FlightServer that uses mohair to receive substrait plans.

    This approach based on click's example, Repo:
        https://github.com/pallets/click/blob/main/examples/repo/repo.py#L43-L54
    """

    # Initialize a context that is passed between CLI commands. The `cli_service_ctx`
    # decorator should be used for the `ServiceContext` instance to be passed to the CLI
    # command or command group.
    ctx.obj = ServiceContext()


#   |> Command group for interacting with storage services as a client
@cli.group()
@click.option('--port', default=GRPC_PORT, show_default=True)
@cli_service_ctx
def client(srv_ctx, port):
    srv_ctx.port = port


#   |> Command group for interacting with storage services (FlightServer impls)
@cli.group()
@click.option('--port', default=GRPC_PORT, show_default=True)
@cli_service_ctx
def service(srv_ctx, port):
    srv_ctx.port = port


#   |> Group (service)
@service.group()
@click.option('--data-path', type=Path, default=DB_FPATH)
@click.option( '--engine'
              ,type=click.Choice(DB_ENGINES, case_sensitive=False)
              ,default=DB_ENGINES[0]
              ,show_default=True)
@cli_service_ctx
def database(srv_ctx, data_path, engine):
    logger.debug(f'DB data path: {data_path}')
    logger.debug(f'Using DBMS engine: {engine}')

    # TODO: make this cleaner
    if engine == DB_ENGINES[0]:
        srv_ctx.service = DatabaseService(
             service_location=srv_ctx.listen_uri()
            ,db_fpath=data_path
        )


# >> Commands

#   |> Client commands
@client.command()
@cli_service_ctx
def list(srv_ctx):
    flight_client = MohairClient.ConnectTo(srv_ctx.connect_uri())
    flight_client.GetFlights()

@client.command()
@click.option('--plan-file', type=str, required=True)
@cli_service_ctx
def send(srv_ctx, plan_file):
    plan_fpath = Path(plan_file)

    if not plan_fpath.is_file():
        sys.exit(f'Invalid file path for query plan: [{plan_file}]')

    with open(plan_fpath, 'rb') as proto_handle:
        message_bytes = proto_handle.read()

    flight_client = MohairClient.ConnectTo(srv_ctx.connect_uri())
    flight_client.SendQueryPlan(message_bytes)


#   |> Service commands (common)
# @service.command()
# def status(srv_ctx):
#     logger.debug(srv_ctx.service.info())
# 
#     if not dry_run:
#         srv_ctx.service.serve()
    

#   |> Service commands (database)
@database.command()
@click.option('--dry-run/--no-dry-run', default=False, show_default=True)
@cli_service_ctx
def start(srv_ctx, dry_run):
    logger.debug(srv_ctx.service.info())

    if not dry_run:
        srv_ctx.service.serve()
