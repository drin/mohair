# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: substrait/type.proto
# Protobuf Python Version: 4.25.2
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from google.protobuf import empty_pb2 as google_dot_protobuf_dot_empty__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x14substrait/type.proto\x12\tsubstrait\x1a\x1bgoogle/protobuf/empty.proto\"\xb4(\n\x04Type\x12-\n\x04\x62ool\x18\x01 \x01(\x0b\x32\x17.substrait.Type.BooleanH\x00R\x04\x62ool\x12$\n\x02i8\x18\x02 \x01(\x0b\x32\x12.substrait.Type.I8H\x00R\x02i8\x12\'\n\x03i16\x18\x03 \x01(\x0b\x32\x13.substrait.Type.I16H\x00R\x03i16\x12\'\n\x03i32\x18\x05 \x01(\x0b\x32\x13.substrait.Type.I32H\x00R\x03i32\x12\'\n\x03i64\x18\x07 \x01(\x0b\x32\x13.substrait.Type.I64H\x00R\x03i64\x12*\n\x04\x66p32\x18\n \x01(\x0b\x32\x14.substrait.Type.FP32H\x00R\x04\x66p32\x12*\n\x04\x66p64\x18\x0b \x01(\x0b\x32\x14.substrait.Type.FP64H\x00R\x04\x66p64\x12\x30\n\x06string\x18\x0c \x01(\x0b\x32\x16.substrait.Type.StringH\x00R\x06string\x12\x30\n\x06\x62inary\x18\r \x01(\x0b\x32\x16.substrait.Type.BinaryH\x00R\x06\x62inary\x12\x39\n\ttimestamp\x18\x0e \x01(\x0b\x32\x19.substrait.Type.TimestampH\x00R\ttimestamp\x12*\n\x04\x64\x61te\x18\x10 \x01(\x0b\x32\x14.substrait.Type.DateH\x00R\x04\x64\x61te\x12*\n\x04time\x18\x11 \x01(\x0b\x32\x14.substrait.Type.TimeH\x00R\x04time\x12\x43\n\rinterval_year\x18\x13 \x01(\x0b\x32\x1c.substrait.Type.IntervalYearH\x00R\x0cintervalYear\x12@\n\x0cinterval_day\x18\x14 \x01(\x0b\x32\x1b.substrait.Type.IntervalDayH\x00R\x0bintervalDay\x12@\n\x0ctimestamp_tz\x18\x1d \x01(\x0b\x32\x1b.substrait.Type.TimestampTZH\x00R\x0btimestampTz\x12*\n\x04uuid\x18  \x01(\x0b\x32\x14.substrait.Type.UUIDH\x00R\x04uuid\x12:\n\nfixed_char\x18\x15 \x01(\x0b\x32\x19.substrait.Type.FixedCharH\x00R\tfixedChar\x12\x33\n\x07varchar\x18\x16 \x01(\x0b\x32\x17.substrait.Type.VarCharH\x00R\x07varchar\x12@\n\x0c\x66ixed_binary\x18\x17 \x01(\x0b\x32\x1b.substrait.Type.FixedBinaryH\x00R\x0b\x66ixedBinary\x12\x33\n\x07\x64\x65\x63imal\x18\x18 \x01(\x0b\x32\x17.substrait.Type.DecimalH\x00R\x07\x64\x65\x63imal\x12\x30\n\x06struct\x18\x19 \x01(\x0b\x32\x16.substrait.Type.StructH\x00R\x06struct\x12*\n\x04list\x18\x1b \x01(\x0b\x32\x14.substrait.Type.ListH\x00R\x04list\x12\'\n\x03map\x18\x1c \x01(\x0b\x32\x13.substrait.Type.MapH\x00R\x03map\x12@\n\x0cuser_defined\x18\x1e \x01(\x0b\x32\x1b.substrait.Type.UserDefinedH\x00R\x0buserDefined\x12\x43\n\x1buser_defined_type_reference\x18\x1f \x01(\rB\x02\x18\x01H\x00R\x18userDefinedTypeReference\x1a\x82\x01\n\x07\x42oolean\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a}\n\x02I8\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a~\n\x03I16\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a~\n\x03I32\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a~\n\x03I64\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x7f\n\x04\x46P32\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x7f\n\x04\x46P64\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x81\x01\n\x06String\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x81\x01\n\x06\x42inary\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x84\x01\n\tTimestamp\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x7f\n\x04\x44\x61te\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x7f\n\x04Time\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x86\x01\n\x0bTimestampTZ\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x87\x01\n\x0cIntervalYear\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x86\x01\n\x0bIntervalDay\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x7f\n\x04UUID\x12\x38\n\x18type_variation_reference\x18\x01 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x02 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x9c\x01\n\tFixedChar\x12\x16\n\x06length\x18\x01 \x01(\x05R\x06length\x12\x38\n\x18type_variation_reference\x18\x02 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x9a\x01\n\x07VarChar\x12\x16\n\x06length\x18\x01 \x01(\x05R\x06length\x12\x38\n\x18type_variation_reference\x18\x02 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\x9e\x01\n\x0b\x46ixedBinary\x12\x16\n\x06length\x18\x01 \x01(\x05R\x06length\x12\x38\n\x18type_variation_reference\x18\x02 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xb6\x01\n\x07\x44\x65\x63imal\x12\x14\n\x05scale\x18\x01 \x01(\x05R\x05scale\x12\x1c\n\tprecision\x18\x02 \x01(\x05R\tprecision\x12\x38\n\x18type_variation_reference\x18\x03 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x04 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xa8\x01\n\x06Struct\x12%\n\x05types\x18\x01 \x03(\x0b\x32\x0f.substrait.TypeR\x05types\x12\x38\n\x18type_variation_reference\x18\x02 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xa4\x01\n\x04List\x12#\n\x04type\x18\x01 \x01(\x0b\x32\x0f.substrait.TypeR\x04type\x12\x38\n\x18type_variation_reference\x18\x02 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xc8\x01\n\x03Map\x12!\n\x03key\x18\x01 \x01(\x0b\x32\x0f.substrait.TypeR\x03key\x12%\n\x05value\x18\x02 \x01(\x0b\x32\x0f.substrait.TypeR\x05value\x12\x38\n\x18type_variation_reference\x18\x03 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x04 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xf1\x01\n\x0bUserDefined\x12%\n\x0etype_reference\x18\x01 \x01(\rR\rtypeReference\x12\x38\n\x18type_variation_reference\x18\x02 \x01(\rR\x16typeVariationReference\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x12\x42\n\x0ftype_parameters\x18\x04 \x03(\x0b\x32\x19.substrait.Type.ParameterR\x0etypeParameters\x1a\xde\x01\n\tParameter\x12,\n\x04null\x18\x01 \x01(\x0b\x32\x16.google.protobuf.EmptyH\x00R\x04null\x12.\n\tdata_type\x18\x02 \x01(\x0b\x32\x0f.substrait.TypeH\x00R\x08\x64\x61taType\x12\x1a\n\x07\x62oolean\x18\x03 \x01(\x08H\x00R\x07\x62oolean\x12\x1a\n\x07integer\x18\x04 \x01(\x03H\x00R\x07integer\x12\x14\n\x04\x65num\x18\x05 \x01(\tH\x00R\x04\x65num\x12\x18\n\x06string\x18\x06 \x01(\tH\x00R\x06stringB\x0b\n\tparameter\"^\n\x0bNullability\x12\x1b\n\x17NULLABILITY_UNSPECIFIED\x10\x00\x12\x18\n\x14NULLABILITY_NULLABLE\x10\x01\x12\x18\n\x14NULLABILITY_REQUIRED\x10\x02\x42\x06\n\x04kind\"S\n\x0bNamedStruct\x12\x14\n\x05names\x18\x01 \x03(\tR\x05names\x12.\n\x06struct\x18\x02 \x01(\x0b\x32\x16.substrait.Type.StructR\x06structB\x8a\x01\n\rcom.substraitB\tTypeProtoP\x01Z*github.com/substrait-io/substrait-go/proto\xa2\x02\x03SXX\xaa\x02\tSubstrait\xca\x02\tSubstrait\xe2\x02\x15Substrait\\GPBMetadata\xea\x02\tSubstraitb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'substrait.type_pb2', _globals)
if _descriptor._USE_C_DESCRIPTORS == False:
  _globals['DESCRIPTOR']._options = None
  _globals['DESCRIPTOR']._serialized_options = b'\n\rcom.substraitB\tTypeProtoP\001Z*github.com/substrait-io/substrait-go/proto\242\002\003SXX\252\002\tSubstrait\312\002\tSubstrait\342\002\025Substrait\\GPBMetadata\352\002\tSubstrait'
  _globals['_TYPE'].fields_by_name['user_defined_type_reference']._options = None
  _globals['_TYPE'].fields_by_name['user_defined_type_reference']._serialized_options = b'\030\001'
  _globals['_TYPE']._serialized_start=65
  _globals['_TYPE']._serialized_end=5237
  _globals['_TYPE_BOOLEAN']._serialized_start=1364
  _globals['_TYPE_BOOLEAN']._serialized_end=1494
  _globals['_TYPE_I8']._serialized_start=1496
  _globals['_TYPE_I8']._serialized_end=1621
  _globals['_TYPE_I16']._serialized_start=1623
  _globals['_TYPE_I16']._serialized_end=1749
  _globals['_TYPE_I32']._serialized_start=1751
  _globals['_TYPE_I32']._serialized_end=1877
  _globals['_TYPE_I64']._serialized_start=1879
  _globals['_TYPE_I64']._serialized_end=2005
  _globals['_TYPE_FP32']._serialized_start=2007
  _globals['_TYPE_FP32']._serialized_end=2134
  _globals['_TYPE_FP64']._serialized_start=2136
  _globals['_TYPE_FP64']._serialized_end=2263
  _globals['_TYPE_STRING']._serialized_start=2266
  _globals['_TYPE_STRING']._serialized_end=2395
  _globals['_TYPE_BINARY']._serialized_start=2398
  _globals['_TYPE_BINARY']._serialized_end=2527
  _globals['_TYPE_TIMESTAMP']._serialized_start=2530
  _globals['_TYPE_TIMESTAMP']._serialized_end=2662
  _globals['_TYPE_DATE']._serialized_start=2664
  _globals['_TYPE_DATE']._serialized_end=2791
  _globals['_TYPE_TIME']._serialized_start=2793
  _globals['_TYPE_TIME']._serialized_end=2920
  _globals['_TYPE_TIMESTAMPTZ']._serialized_start=2923
  _globals['_TYPE_TIMESTAMPTZ']._serialized_end=3057
  _globals['_TYPE_INTERVALYEAR']._serialized_start=3060
  _globals['_TYPE_INTERVALYEAR']._serialized_end=3195
  _globals['_TYPE_INTERVALDAY']._serialized_start=3198
  _globals['_TYPE_INTERVALDAY']._serialized_end=3332
  _globals['_TYPE_UUID']._serialized_start=3334
  _globals['_TYPE_UUID']._serialized_end=3461
  _globals['_TYPE_FIXEDCHAR']._serialized_start=3464
  _globals['_TYPE_FIXEDCHAR']._serialized_end=3620
  _globals['_TYPE_VARCHAR']._serialized_start=3623
  _globals['_TYPE_VARCHAR']._serialized_end=3777
  _globals['_TYPE_FIXEDBINARY']._serialized_start=3780
  _globals['_TYPE_FIXEDBINARY']._serialized_end=3938
  _globals['_TYPE_DECIMAL']._serialized_start=3941
  _globals['_TYPE_DECIMAL']._serialized_end=4123
  _globals['_TYPE_STRUCT']._serialized_start=4126
  _globals['_TYPE_STRUCT']._serialized_end=4294
  _globals['_TYPE_LIST']._serialized_start=4297
  _globals['_TYPE_LIST']._serialized_end=4461
  _globals['_TYPE_MAP']._serialized_start=4464
  _globals['_TYPE_MAP']._serialized_end=4664
  _globals['_TYPE_USERDEFINED']._serialized_start=4667
  _globals['_TYPE_USERDEFINED']._serialized_end=4908
  _globals['_TYPE_PARAMETER']._serialized_start=4911
  _globals['_TYPE_PARAMETER']._serialized_end=5133
  _globals['_TYPE_NULLABILITY']._serialized_start=5135
  _globals['_TYPE_NULLABILITY']._serialized_end=5229
  _globals['_NAMEDSTRUCT']._serialized_start=5239
  _globals['_NAMEDSTRUCT']._serialized_end=5322
# @@protoc_insertion_point(module_scope)
