# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# NO CHECKED-IN PROTOBUF GENCODE
# source: skytether/substrait/extensions/extensions.proto
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
    'skytether/substrait/extensions/extensions.proto'
)
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from google.protobuf import any_pb2 as google_dot_protobuf_dot_any__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n/skytether/substrait/extensions/extensions.proto\x12\x1eskytether.substrait.extensions\x1a\x19google/protobuf/any.proto\"X\n\x12SimpleExtensionURI\x12\x30\n\x14\x65xtension_uri_anchor\x18\x01 \x01(\rR\x12\x65xtensionUriAnchor\x12\x10\n\x03uri\x18\x02 \x01(\tR\x03uri\"\xd2\x06\n\x1aSimpleExtensionDeclaration\x12q\n\x0e\x65xtension_type\x18\x01 \x01(\x0b\x32H.skytether.substrait.extensions.SimpleExtensionDeclaration.ExtensionTypeH\x00R\rextensionType\x12\x8d\x01\n\x18\x65xtension_type_variation\x18\x02 \x01(\x0b\x32Q.skytether.substrait.extensions.SimpleExtensionDeclaration.ExtensionTypeVariationH\x00R\x16\x65xtensionTypeVariation\x12}\n\x12\x65xtension_function\x18\x03 \x01(\x0b\x32L.skytether.substrait.extensions.SimpleExtensionDeclaration.ExtensionFunctionH\x00R\x11\x65xtensionFunction\x1a|\n\rExtensionType\x12\x36\n\x17\x65xtension_uri_reference\x18\x01 \x01(\rR\x15\x65xtensionUriReference\x12\x1f\n\x0btype_anchor\x18\x02 \x01(\rR\ntypeAnchor\x12\x12\n\x04name\x18\x03 \x01(\tR\x04name\x1a\x98\x01\n\x16\x45xtensionTypeVariation\x12\x36\n\x17\x65xtension_uri_reference\x18\x01 \x01(\rR\x15\x65xtensionUriReference\x12\x32\n\x15type_variation_anchor\x18\x02 \x01(\rR\x13typeVariationAnchor\x12\x12\n\x04name\x18\x03 \x01(\tR\x04name\x1a\x88\x01\n\x11\x45xtensionFunction\x12\x36\n\x17\x65xtension_uri_reference\x18\x01 \x01(\rR\x15\x65xtensionUriReference\x12\'\n\x0f\x66unction_anchor\x18\x02 \x01(\rR\x0e\x66unctionAnchor\x12\x12\n\x04name\x18\x03 \x01(\tR\x04nameB\x0e\n\x0cmapping_type\"\x85\x01\n\x11\x41\x64vancedExtension\x12\x38\n\x0coptimization\x18\x01 \x03(\x0b\x32\x14.google.protobuf.AnyR\x0coptimization\x12\x36\n\x0b\x65nhancement\x18\x02 \x01(\x0b\x32\x14.google.protobuf.AnyR\x0b\x65nhancementB\xcf\x01\n\"com.skytether.substrait.extensionsB\x0f\x45xtensionsProtoP\x01\xa2\x02\x03SSE\xaa\x02\x1eSkytether.Substrait.Extensions\xca\x02\x1eSkytether\\Substrait\\Extensions\xe2\x02*Skytether\\Substrait\\Extensions\\GPBMetadata\xea\x02 Skytether::Substrait::Extensionsb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'skytether.substrait.extensions.extensions_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  _globals['DESCRIPTOR']._loaded_options = None
  _globals['DESCRIPTOR']._serialized_options = b'\n\"com.skytether.substrait.extensionsB\017ExtensionsProtoP\001\242\002\003SSE\252\002\036Skytether.Substrait.Extensions\312\002\036Skytether\\Substrait\\Extensions\342\002*Skytether\\Substrait\\Extensions\\GPBMetadata\352\002 Skytether::Substrait::Extensions'
  _globals['_SIMPLEEXTENSIONURI']._serialized_start=110
  _globals['_SIMPLEEXTENSIONURI']._serialized_end=198
  _globals['_SIMPLEEXTENSIONDECLARATION']._serialized_start=201
  _globals['_SIMPLEEXTENSIONDECLARATION']._serialized_end=1051
  _globals['_SIMPLEEXTENSIONDECLARATION_EXTENSIONTYPE']._serialized_start=617
  _globals['_SIMPLEEXTENSIONDECLARATION_EXTENSIONTYPE']._serialized_end=741
  _globals['_SIMPLEEXTENSIONDECLARATION_EXTENSIONTYPEVARIATION']._serialized_start=744
  _globals['_SIMPLEEXTENSIONDECLARATION_EXTENSIONTYPEVARIATION']._serialized_end=896
  _globals['_SIMPLEEXTENSIONDECLARATION_EXTENSIONFUNCTION']._serialized_start=899
  _globals['_SIMPLEEXTENSIONDECLARATION_EXTENSIONFUNCTION']._serialized_end=1035
  _globals['_ADVANCEDEXTENSION']._serialized_start=1054
  _globals['_ADVANCEDEXTENSION']._serialized_end=1187
# @@protoc_insertion_point(module_scope)