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
Classes and functions to insulate Mohair from substrait implementation details.
"""


# ------------------------------
# Dependencies

# >> Standard libs
from typing import TypeAlias
from dataclasses import dataclass

# >> substrait types
from mohair.substrait.plan_pb2 import Plan, PlanRel

# >> internal
#   |> classes
from mohair.query.types import MohairPlan, DecomposedPlan
from mohair.query.plans import PlanExplorer

#   |> functions
from mohair import CreateMohairLogger
from mohair.query.operators import MohairFrom, SubstraitFrom, PlanAnchorFrom


# ------------------------------
# Module Variables

# >> Logging
logger = CreateMohairLogger(__name__)

# >> Type aliases
PlanTypeAlias: TypeAlias = 'SubstraitPlan'


# ------------------------------
# Classes

@dataclass
class SubstraitPlan:
    """
    Wrapper class that references a serialized substrait plan and the root of its
    deserialized representation.
    """

    msg : bytes
    plan: Plan

    plan_rootrel: PlanRel = None
    plan_rootndx: int     = -1

    # TODO: do some hashing to identify the plan for materialized views
    # @classmethod
    # def FromBytes(cls, plan_bytes: bytes, signed: bool=True) -> int:
    #     return int.from_bytes(plan_bytes, byteorder='big', signed=signed)

    # @classmethod
    # def ToBytes(cls, plan_hash: int, width: int=8, signed: bool=True) -> bytes:
    #     return plan_hash.to_bytes(width, byteorder='big', signed=signed)

    @classmethod
    def FromMessageBytes(cls, substrait_msg: bytes) -> PlanTypeAlias:
        substrait_plan = Plan()
        substrait_plan.ParseFromString(substrait_msg)

        return cls(substrait_msg, substrait_plan)

    @classmethod
    def FromMessageProto(cls, substrait_proto: Plan) -> PlanTypeAlias:
        substrait_msg = substrait_proto.SerializeToString()
        return cls(substrait_msg, substrait_proto)

    def CopyPlan(self) -> Plan:
        new_plan = Plan()
        new_plan.CopyFrom(self.plan)

        return new_plan

    def ToMohair(self) -> MohairPlan:
        mohair_root = None
        root_rel    = None
        root_ndx    = -1

        for plan_ndx, plan_rel in enumerate(self.plan.relations):
            if plan_rel.HasField('root'):
                # A query plan should have only 1 `root`, even if it has many trees
                assert mohair_root == None

                logger.debug(f'translating plan {plan_ndx}')

                mohair_root = MohairFrom(plan_rel.root.input)
                root_rel    = plan_rel
                root_ndx    = plan_ndx

        # Very confusing if a query plan did not have any `root`
        assert mohair_root is not None

        # Cache the root operator while we're here
        self.plan_rootrel = root_rel
        self.plan_rootndx = root_ndx

        return PlanExplorer.WalkPlan(mohair_root)

    def ToSubPlanMessage(self, dplan: DecomposedPlan, subplan_ndx: int) -> PlanTypeAlias:
        if self.plan_rootrel.root.input is dplan.query_plan.plan_root.plan_op:
            logger.error('Invalid subplan for message')
            return None

        # prepare the protobuf; we make copies to optimize making messages
        subplan_message = self.CopyPlan()

        # replace the RelRoot input of the new message
        subplan_newroot = dplan.anchor_subplans[subplan_ndx]
        subplan_oldroot = subplan_message.relations[self.plan_rootndx]

        subplan_oldroot.root.input.CopyFrom(SubstraitFrom(subplan_newroot))

        # include a PlanAnchor message
        #   get a `PlanAnchor` substrait message
        superplan_anchor = PlanAnchorFrom(dplan.anchor_root.plan_root)
        #   then pack it into the `optimization` property (an Any message)
        subplan_message.advanced_extensions.optimization.Pack(superplan_anchor)

        # return a new instance that also serializes the proto structure
        return self.__class__.FromMessageProto(subplan_message)
