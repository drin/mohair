# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: skytether/algebra.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import builder as _builder
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from google.protobuf import any_pb2 as google_dot_protobuf_dot_any__pb2
from mohair.substrait.extensions import extensions_pb2 as substrait_dot_extensions_dot_extensions__pb2
from mohair.substrait import type_pb2 as substrait_dot_type__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x17skytether/algebra.proto\x12\tskytether\x1a\x19google/protobuf/any.proto\x1a%substrait/extensions/extensions.proto\x1a\x14substrait/type.proto\"V\n\x06SkyRel\x12\x16\n\x06\x64omain\x18\x01 \x01(\tR\x06\x64omain\x12\x1c\n\tpartition\x18\x02 \x01(\tR\tpartition\x12\x16\n\x06slices\x18\x03 \x03(\rR\x06slices\"Z\n\nKineticRel\x12\x16\n\x06\x64omain\x18\x01 \x01(\tR\x06\x64omain\x12\x1c\n\tpartition\x18\x02 \x01(\tR\tpartition\x12\x16\n\x06slices\x18\x03 \x03(\rR\x06slicesb\x06proto3')

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'skytether.algebra_pb2', globals())
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _SKYREL._serialized_start=126
  _SKYREL._serialized_end=212
  _KINETICREL._serialized_start=214
  _KINETICREL._serialized_end=304
# @@protoc_insertion_point(module_scope)
