# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# NO CHECKED-IN PROTOBUF GENCODE
# source: substrait/capabilities.proto
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
    'substrait/capabilities.proto'
)
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x1csubstrait/capabilities.proto\x12\tsubstrait\"\xec\x02\n\x0c\x43\x61pabilities\x12-\n\x12substrait_versions\x18\x01 \x03(\tR\x11substraitVersions\x12?\n\x1c\x61\x64vanced_extension_type_urls\x18\x02 \x03(\tR\x19\x61\x64vancedExtensionTypeUrls\x12T\n\x11simple_extensions\x18\x03 \x03(\x0b\x32\'.substrait.Capabilities.SimpleExtensionR\x10simpleExtensions\x1a\x95\x01\n\x0fSimpleExtension\x12\x10\n\x03uri\x18\x01 \x01(\tR\x03uri\x12#\n\rfunction_keys\x18\x02 \x03(\tR\x0c\x66unctionKeys\x12\x1b\n\ttype_keys\x18\x03 \x03(\tR\x08typeKeys\x12.\n\x13type_variation_keys\x18\x04 \x03(\tR\x11typeVariationKeysB\x92\x01\n\rcom.substraitB\x11\x43\x61pabilitiesProtoP\x01Z*github.com/substrait-io/substrait-go/proto\xa2\x02\x03SXX\xaa\x02\tSubstrait\xca\x02\tSubstrait\xe2\x02\x15Substrait\\GPBMetadata\xea\x02\tSubstraitb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'substrait.capabilities_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  _globals['DESCRIPTOR']._loaded_options = None
  _globals['DESCRIPTOR']._serialized_options = b'\n\rcom.substraitB\021CapabilitiesProtoP\001Z*github.com/substrait-io/substrait-go/proto\242\002\003SXX\252\002\tSubstrait\312\002\tSubstrait\342\002\025Substrait\\GPBMetadata\352\002\tSubstrait'
  _globals['_CAPABILITIES']._serialized_start=44
  _globals['_CAPABILITIES']._serialized_end=408
  _globals['_CAPABILITIES_SIMPLEEXTENSION']._serialized_start=259
  _globals['_CAPABILITIES_SIMPLEEXTENSION']._serialized_end=408
# @@protoc_insertion_point(module_scope)
