# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# NO CHECKED-IN PROTOBUF GENCODE
# source: skytether/substrait/parameterized_types.proto
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
    'skytether/substrait/parameterized_types.proto'
)
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from mohair.skytether.substrait import type_pb2 as skytether_dot_substrait_dot_type__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n-skytether/substrait/parameterized_types.proto\x12\x13skytether.substrait\x1a\x1eskytether/substrait/type.proto\"\xbf,\n\x11ParameterizedType\x12\x37\n\x04\x62ool\x18\x01 \x01(\x0b\x32!.skytether.substrait.Type.BooleanH\x00R\x04\x62ool\x12.\n\x02i8\x18\x02 \x01(\x0b\x32\x1c.skytether.substrait.Type.I8H\x00R\x02i8\x12\x31\n\x03i16\x18\x03 \x01(\x0b\x32\x1d.skytether.substrait.Type.I16H\x00R\x03i16\x12\x31\n\x03i32\x18\x05 \x01(\x0b\x32\x1d.skytether.substrait.Type.I32H\x00R\x03i32\x12\x31\n\x03i64\x18\x07 \x01(\x0b\x32\x1d.skytether.substrait.Type.I64H\x00R\x03i64\x12\x34\n\x04\x66p32\x18\n \x01(\x0b\x32\x1e.skytether.substrait.Type.FP32H\x00R\x04\x66p32\x12\x34\n\x04\x66p64\x18\x0b \x01(\x0b\x32\x1e.skytether.substrait.Type.FP64H\x00R\x04\x66p64\x12:\n\x06string\x18\x0c \x01(\x0b\x32 .skytether.substrait.Type.StringH\x00R\x06string\x12:\n\x06\x62inary\x18\r \x01(\x0b\x32 .skytether.substrait.Type.BinaryH\x00R\x06\x62inary\x12G\n\ttimestamp\x18\x0e \x01(\x0b\x32#.skytether.substrait.Type.TimestampB\x02\x18\x01H\x00R\ttimestamp\x12\x34\n\x04\x64\x61te\x18\x10 \x01(\x0b\x32\x1e.skytether.substrait.Type.DateH\x00R\x04\x64\x61te\x12\x34\n\x04time\x18\x11 \x01(\x0b\x32\x1e.skytether.substrait.Type.TimeH\x00R\x04time\x12M\n\rinterval_year\x18\x13 \x01(\x0b\x32&.skytether.substrait.Type.IntervalYearH\x00R\x0cintervalYear\x12\x64\n\x0cinterval_day\x18\x14 \x01(\x0b\x32?.skytether.substrait.ParameterizedType.ParameterizedIntervalDayH\x00R\x0bintervalDay\x12s\n\x11interval_compound\x18$ \x01(\x0b\x32\x44.skytether.substrait.ParameterizedType.ParameterizedIntervalCompoundH\x00R\x10intervalCompound\x12N\n\x0ctimestamp_tz\x18\x1d \x01(\x0b\x32%.skytether.substrait.Type.TimestampTZB\x02\x18\x01H\x00R\x0btimestampTz\x12\x34\n\x04uuid\x18  \x01(\x0b\x32\x1e.skytether.substrait.Type.UUIDH\x00R\x04uuid\x12^\n\nfixed_char\x18\x15 \x01(\x0b\x32=.skytether.substrait.ParameterizedType.ParameterizedFixedCharH\x00R\tfixedChar\x12W\n\x07varchar\x18\x16 \x01(\x0b\x32;.skytether.substrait.ParameterizedType.ParameterizedVarCharH\x00R\x07varchar\x12\x64\n\x0c\x66ixed_binary\x18\x17 \x01(\x0b\x32?.skytether.substrait.ParameterizedType.ParameterizedFixedBinaryH\x00R\x0b\x66ixedBinary\x12W\n\x07\x64\x65\x63imal\x18\x18 \x01(\x0b\x32;.skytether.substrait.ParameterizedType.ParameterizedDecimalH\x00R\x07\x64\x65\x63imal\x12y\n\x13precision_timestamp\x18\" \x01(\x0b\x32\x46.skytether.substrait.ParameterizedType.ParameterizedPrecisionTimestampH\x00R\x12precisionTimestamp\x12\x80\x01\n\x16precision_timestamp_tz\x18# \x01(\x0b\x32H.skytether.substrait.ParameterizedType.ParameterizedPrecisionTimestampTZH\x00R\x14precisionTimestampTz\x12T\n\x06struct\x18\x19 \x01(\x0b\x32:.skytether.substrait.ParameterizedType.ParameterizedStructH\x00R\x06struct\x12N\n\x04list\x18\x1b \x01(\x0b\x32\x38.skytether.substrait.ParameterizedType.ParameterizedListH\x00R\x04list\x12K\n\x03map\x18\x1c \x01(\x0b\x32\x37.skytether.substrait.ParameterizedType.ParameterizedMapH\x00R\x03map\x12\x64\n\x0cuser_defined\x18\x1e \x01(\x0b\x32?.skytether.substrait.ParameterizedType.ParameterizedUserDefinedH\x00R\x0buserDefined\x12\x36\n\x14user_defined_pointer\x18\x1f \x01(\rB\x02\x18\x01H\x00R\x12userDefinedPointer\x12]\n\x0etype_parameter\x18! \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.TypeParameterH\x00R\rtypeParameter\x1a\x63\n\rTypeParameter\x12\x12\n\x04name\x18\x01 \x01(\tR\x04name\x12>\n\x06\x62ounds\x18\x02 \x03(\x0b\x32&.skytether.substrait.ParameterizedTypeR\x06\x62ounds\x1a\xfa\x01\n\x10IntegerParameter\x12\x12\n\x04name\x18\x01 \x01(\tR\x04name\x12j\n\x15range_start_inclusive\x18\x02 \x01(\x0b\x32\x36.skytether.substrait.ParameterizedType.NullableIntegerR\x13rangeStartInclusive\x12\x66\n\x13range_end_exclusive\x18\x03 \x01(\x0b\x32\x36.skytether.substrait.ParameterizedType.NullableIntegerR\x11rangeEndExclusive\x1a\'\n\x0fNullableInteger\x12\x14\n\x05value\x18\x01 \x01(\x03R\x05value\x1a\xdc\x01\n\x16ParameterizedFixedChar\x12L\n\x06length\x18\x01 \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.IntegerOptionR\x06length\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\xda\x01\n\x14ParameterizedVarChar\x12L\n\x06length\x18\x01 \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.IntegerOptionR\x06length\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\xde\x01\n\x18ParameterizedFixedBinary\x12L\n\x06length\x18\x01 \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.IntegerOptionR\x06length\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\xac\x02\n\x14ParameterizedDecimal\x12J\n\x05scale\x18\x01 \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.IntegerOptionR\x05scale\x12R\n\tprecision\x18\x02 \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.IntegerOptionR\tprecision\x12+\n\x11variation_pointer\x18\x03 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x04 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\xe4\x01\n\x18ParameterizedIntervalDay\x12R\n\tprecision\x18\x01 \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.IntegerOptionR\tprecision\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\xe9\x01\n\x1dParameterizedIntervalCompound\x12R\n\tprecision\x18\x01 \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.IntegerOptionR\tprecision\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\xeb\x01\n\x1fParameterizedPrecisionTimestamp\x12R\n\tprecision\x18\x01 \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.IntegerOptionR\tprecision\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\xed\x01\n!ParameterizedPrecisionTimestampTZ\x12R\n\tprecision\x18\x01 \x01(\x0b\x32\x34.skytether.substrait.ParameterizedType.IntegerOptionR\tprecision\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\xc9\x01\n\x13ParameterizedStruct\x12<\n\x05types\x18\x01 \x03(\x0b\x32&.skytether.substrait.ParameterizedTypeR\x05types\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\x84\x01\n\x18ParameterizedNamedStruct\x12\x14\n\x05names\x18\x01 \x03(\tR\x05names\x12R\n\x06struct\x18\x02 \x01(\x0b\x32:.skytether.substrait.ParameterizedType.ParameterizedStructR\x06struct\x1a\xc5\x01\n\x11ParameterizedList\x12:\n\x04type\x18\x01 \x01(\x0b\x32&.skytether.substrait.ParameterizedTypeR\x04type\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\x80\x02\n\x10ParameterizedMap\x12\x38\n\x03key\x18\x01 \x01(\x0b\x32&.skytether.substrait.ParameterizedTypeR\x03key\x12<\n\x05value\x18\x02 \x01(\x0b\x32&.skytether.substrait.ParameterizedTypeR\x05value\x12+\n\x11variation_pointer\x18\x03 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x04 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\xb3\x01\n\x18ParameterizedUserDefined\x12!\n\x0ctype_pointer\x18\x01 \x01(\rR\x0btypePointer\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12G\n\x0bnullability\x18\x03 \x01(\x0e\x32%.skytether.substrait.Type.NullabilityR\x0bnullability\x1a\x94\x01\n\rIntegerOption\x12\x1a\n\x07literal\x18\x01 \x01(\x05H\x00R\x07literal\x12W\n\tparameter\x18\x02 \x01(\x0b\x32\x37.skytether.substrait.ParameterizedType.IntegerParameterH\x00R\tparameterB\x0e\n\x0cinteger_typeB\x06\n\x04kindB\x9f\x01\n\x17\x63om.skytether.substraitB\x17ParameterizedTypesProtoP\x01\xa2\x02\x03SSX\xaa\x02\x13Skytether.Substrait\xca\x02\x13Skytether\\Substrait\xe2\x02\x1fSkytether\\Substrait\\GPBMetadata\xea\x02\x14Skytether::Substraitb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'skytether.substrait.parameterized_types_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  _globals['DESCRIPTOR']._loaded_options = None
  _globals['DESCRIPTOR']._serialized_options = b'\n\027com.skytether.substraitB\027ParameterizedTypesProtoP\001\242\002\003SSX\252\002\023Skytether.Substrait\312\002\023Skytether\\Substrait\342\002\037Skytether\\Substrait\\GPBMetadata\352\002\024Skytether::Substrait'
  _globals['_PARAMETERIZEDTYPE'].fields_by_name['timestamp']._loaded_options = None
  _globals['_PARAMETERIZEDTYPE'].fields_by_name['timestamp']._serialized_options = b'\030\001'
  _globals['_PARAMETERIZEDTYPE'].fields_by_name['timestamp_tz']._loaded_options = None
  _globals['_PARAMETERIZEDTYPE'].fields_by_name['timestamp_tz']._serialized_options = b'\030\001'
  _globals['_PARAMETERIZEDTYPE'].fields_by_name['user_defined_pointer']._loaded_options = None
  _globals['_PARAMETERIZEDTYPE'].fields_by_name['user_defined_pointer']._serialized_options = b'\030\001'
  _globals['_PARAMETERIZEDTYPE']._serialized_start=103
  _globals['_PARAMETERIZEDTYPE']._serialized_end=5798
  _globals['_PARAMETERIZEDTYPE_TYPEPARAMETER']._serialized_start=2349
  _globals['_PARAMETERIZEDTYPE_TYPEPARAMETER']._serialized_end=2448
  _globals['_PARAMETERIZEDTYPE_INTEGERPARAMETER']._serialized_start=2451
  _globals['_PARAMETERIZEDTYPE_INTEGERPARAMETER']._serialized_end=2701
  _globals['_PARAMETERIZEDTYPE_NULLABLEINTEGER']._serialized_start=2703
  _globals['_PARAMETERIZEDTYPE_NULLABLEINTEGER']._serialized_end=2742
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDFIXEDCHAR']._serialized_start=2745
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDFIXEDCHAR']._serialized_end=2965
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDVARCHAR']._serialized_start=2968
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDVARCHAR']._serialized_end=3186
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDFIXEDBINARY']._serialized_start=3189
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDFIXEDBINARY']._serialized_end=3411
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDDECIMAL']._serialized_start=3414
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDDECIMAL']._serialized_end=3714
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDINTERVALDAY']._serialized_start=3717
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDINTERVALDAY']._serialized_end=3945
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDINTERVALCOMPOUND']._serialized_start=3948
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDINTERVALCOMPOUND']._serialized_end=4181
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDPRECISIONTIMESTAMP']._serialized_start=4184
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDPRECISIONTIMESTAMP']._serialized_end=4419
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDPRECISIONTIMESTAMPTZ']._serialized_start=4422
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDPRECISIONTIMESTAMPTZ']._serialized_end=4659
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDSTRUCT']._serialized_start=4662
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDSTRUCT']._serialized_end=4863
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDNAMEDSTRUCT']._serialized_start=4866
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDNAMEDSTRUCT']._serialized_end=4998
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDLIST']._serialized_start=5001
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDLIST']._serialized_end=5198
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDMAP']._serialized_start=5201
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDMAP']._serialized_end=5457
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDUSERDEFINED']._serialized_start=5460
  _globals['_PARAMETERIZEDTYPE_PARAMETERIZEDUSERDEFINED']._serialized_end=5639
  _globals['_PARAMETERIZEDTYPE_INTEGEROPTION']._serialized_start=5642
  _globals['_PARAMETERIZEDTYPE_INTEGEROPTION']._serialized_end=5790
# @@protoc_insertion_point(module_scope)