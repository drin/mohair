# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# NO CHECKED-IN PROTOBUF GENCODE
# source: substrait/plan.proto
# Protobuf Python Version: 5.28.2
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import runtime_version as _runtime_version
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
_runtime_version.ValidateProtobufRuntimeVersion(
    _runtime_version.Domain.PUBLIC,
    5,
    28,
    2,
    '',
    'substrait/plan.proto'
)
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from mohair.substrait import algebra_pb2 as substrait_dot_algebra__pb2
from mohair.substrait.extensions import extensions_pb2 as substrait_dot_extensions_dot_extensions__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x14substrait/plan.proto\x12\tsubstrait\x1a\x17substrait/algebra.proto\x1a%substrait/extensions/extensions.proto\"c\n\x07PlanRel\x12\"\n\x03rel\x18\x01 \x01(\x0b\x32\x0e.substrait.RelH\x00R\x03rel\x12(\n\x04root\x18\x02 \x01(\x0b\x32\x12.substrait.RelRootH\x00R\x04rootB\n\n\x08rel_type\"\x91\x03\n\x04Plan\x12,\n\x07version\x18\x06 \x01(\x0b\x32\x12.substrait.VersionR\x07version\x12O\n\x0e\x65xtension_uris\x18\x01 \x03(\x0b\x32(.substrait.extensions.SimpleExtensionURIR\rextensionUris\x12P\n\nextensions\x18\x02 \x03(\x0b\x32\x30.substrait.extensions.SimpleExtensionDeclarationR\nextensions\x12\x30\n\trelations\x18\x03 \x03(\x0b\x32\x12.substrait.PlanRelR\trelations\x12X\n\x13\x61\x64vanced_extensions\x18\x04 \x01(\x0b\x32\'.substrait.extensions.AdvancedExtensionR\x12\x61\x64vancedExtensions\x12,\n\x12\x65xpected_type_urls\x18\x05 \x03(\tR\x10\x65xpectedTypeUrls\";\n\x0bPlanVersion\x12,\n\x07version\x18\x06 \x01(\x0b\x32\x12.substrait.VersionR\x07version\"\xa9\x01\n\x07Version\x12!\n\x0cmajor_number\x18\x01 \x01(\rR\x0bmajorNumber\x12!\n\x0cminor_number\x18\x02 \x01(\rR\x0bminorNumber\x12!\n\x0cpatch_number\x18\x03 \x01(\rR\x0bpatchNumber\x12\x19\n\x08git_hash\x18\x04 \x01(\tR\x07gitHash\x12\x1a\n\x08producer\x18\x05 \x01(\tR\x08producerB\x8a\x01\n\rcom.substraitB\tPlanProtoP\x01Z*github.com/substrait-io/substrait-go/proto\xa2\x02\x03SXX\xaa\x02\tSubstrait\xca\x02\tSubstrait\xe2\x02\x15Substrait\\GPBMetadata\xea\x02\tSubstraitb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'substrait.plan_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  _globals['DESCRIPTOR']._loaded_options = None
  _globals['DESCRIPTOR']._serialized_options = b'\n\rcom.substraitB\tPlanProtoP\001Z*github.com/substrait-io/substrait-go/proto\242\002\003SXX\252\002\tSubstrait\312\002\tSubstrait\342\002\025Substrait\\GPBMetadata\352\002\tSubstrait'
  _globals['_PLANREL']._serialized_start=99
  _globals['_PLANREL']._serialized_end=198
  _globals['_PLAN']._serialized_start=201
  _globals['_PLAN']._serialized_end=602
  _globals['_PLANVERSION']._serialized_start=604
  _globals['_PLANVERSION']._serialized_end=663
  _globals['_VERSION']._serialized_start=666
  _globals['_VERSION']._serialized_end=835
# @@protoc_insertion_point(module_scope)
