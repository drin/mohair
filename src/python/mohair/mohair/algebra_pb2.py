# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: mohair/algebra.proto
# Protobuf Python Version: 4.25.0
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from google.protobuf import any_pb2 as google_dot_protobuf_dot_any__pb2
from mohair.substrait import algebra_pb2 as substrait_dot_algebra__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x14mohair/algebra.proto\x12\x06mohair\x1a\x19google/protobuf/any.proto\x1a\x17substrait/algebra.proto\"F\n\x0e\x45xecutionStats\x12\x1a\n\x08\x65xecuted\x18\x01 \x01(\x08R\x08\x65xecuted\x12\x18\n\x07runtime\x18\x02 \x01(\x01R\x07runtime\"\x8c\x01\n\x06SkyRel\x12\x16\n\x06\x64omain\x18\x01 \x01(\tR\x06\x64omain\x12\x1c\n\tpartition\x18\x02 \x01(\tR\tpartition\x12\x16\n\x06slices\x18\x03 \x03(\rR\x06slices\x12\x34\n\texecstats\x18\x04 \x01(\x0b\x32\x16.mohair.ExecutionStatsR\texecstats\"u\n\x06\x45rrRel\x12\x17\n\x07\x65rr_msg\x18\x01 \x01(\tR\x06\x65rrMsg\x12\x31\n\x08\x65rr_code\x18\x02 \x01(\x0e\x32\x16.mohair.ErrRel.ErrTypeR\x07\x65rrCode\"\x1f\n\x07\x45rrType\x12\x14\n\x10INVALID_MSG_TYPE\x10\x00\" \n\x08QueryRel\x12\x14\n\x05query\x18\x01 \x01(\x0cR\x05query\";\n\nPlanAnchor\x12-\n\nanchor_rel\x18\x01 \x01(\x0b\x32\x0e.substrait.RelR\tanchorRelBR\n\ncom.mohairB\x0c\x41lgebraProtoP\x01\xa2\x02\x03MXX\xaa\x02\x06Mohair\xca\x02\x06Mohair\xe2\x02\x12Mohair\\GPBMetadata\xea\x02\x06Mohairb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'mohair.algebra_pb2', _globals)
if _descriptor._USE_C_DESCRIPTORS == False:
  _globals['DESCRIPTOR']._options = None
  _globals['DESCRIPTOR']._serialized_options = b'\n\ncom.mohairB\014AlgebraProtoP\001\242\002\003MXX\252\002\006Mohair\312\002\006Mohair\342\002\022Mohair\\GPBMetadata\352\002\006Mohair'
  _globals['_EXECUTIONSTATS']._serialized_start=84
  _globals['_EXECUTIONSTATS']._serialized_end=154
  _globals['_SKYREL']._serialized_start=157
  _globals['_SKYREL']._serialized_end=297
  _globals['_ERRREL']._serialized_start=299
  _globals['_ERRREL']._serialized_end=416
  _globals['_ERRREL_ERRTYPE']._serialized_start=385
  _globals['_ERRREL_ERRTYPE']._serialized_end=416
  _globals['_QUERYREL']._serialized_start=418
  _globals['_QUERYREL']._serialized_end=450
  _globals['_PLANANCHOR']._serialized_start=452
  _globals['_PLANANCHOR']._serialized_end=511
# @@protoc_insertion_point(module_scope)
