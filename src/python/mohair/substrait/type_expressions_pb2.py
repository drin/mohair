# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: substrait/type_expressions.proto
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from substrait import type_pb2 as substrait_dot_type__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n substrait/type_expressions.proto\x12\tsubstrait\x1a\x14substrait/type.proto\"\xcc&\n\x14\x44\x65rivationExpression\x12-\n\x04\x62ool\x18\x01 \x01(\x0b\x32\x17.substrait.Type.BooleanH\x00R\x04\x62ool\x12$\n\x02i8\x18\x02 \x01(\x0b\x32\x12.substrait.Type.I8H\x00R\x02i8\x12\'\n\x03i16\x18\x03 \x01(\x0b\x32\x13.substrait.Type.I16H\x00R\x03i16\x12\'\n\x03i32\x18\x05 \x01(\x0b\x32\x13.substrait.Type.I32H\x00R\x03i32\x12\'\n\x03i64\x18\x07 \x01(\x0b\x32\x13.substrait.Type.I64H\x00R\x03i64\x12*\n\x04\x66p32\x18\n \x01(\x0b\x32\x14.substrait.Type.FP32H\x00R\x04\x66p32\x12*\n\x04\x66p64\x18\x0b \x01(\x0b\x32\x14.substrait.Type.FP64H\x00R\x04\x66p64\x12\x30\n\x06string\x18\x0c \x01(\x0b\x32\x16.substrait.Type.StringH\x00R\x06string\x12\x30\n\x06\x62inary\x18\r \x01(\x0b\x32\x16.substrait.Type.BinaryH\x00R\x06\x62inary\x12\x39\n\ttimestamp\x18\x0e \x01(\x0b\x32\x19.substrait.Type.TimestampH\x00R\ttimestamp\x12*\n\x04\x64\x61te\x18\x10 \x01(\x0b\x32\x14.substrait.Type.DateH\x00R\x04\x64\x61te\x12*\n\x04time\x18\x11 \x01(\x0b\x32\x14.substrait.Type.TimeH\x00R\x04time\x12\x43\n\rinterval_year\x18\x13 \x01(\x0b\x32\x1c.substrait.Type.IntervalYearH\x00R\x0cintervalYear\x12@\n\x0cinterval_day\x18\x14 \x01(\x0b\x32\x1b.substrait.Type.IntervalDayH\x00R\x0bintervalDay\x12@\n\x0ctimestamp_tz\x18\x1d \x01(\x0b\x32\x1b.substrait.Type.TimestampTZH\x00R\x0btimestampTz\x12*\n\x04uuid\x18  \x01(\x0b\x32\x14.substrait.Type.UUIDH\x00R\x04uuid\x12T\n\nfixed_char\x18\x15 \x01(\x0b\x32\x33.substrait.DerivationExpression.ExpressionFixedCharH\x00R\tfixedChar\x12M\n\x07varchar\x18\x16 \x01(\x0b\x32\x31.substrait.DerivationExpression.ExpressionVarCharH\x00R\x07varchar\x12Z\n\x0c\x66ixed_binary\x18\x17 \x01(\x0b\x32\x35.substrait.DerivationExpression.ExpressionFixedBinaryH\x00R\x0b\x66ixedBinary\x12M\n\x07\x64\x65\x63imal\x18\x18 \x01(\x0b\x32\x31.substrait.DerivationExpression.ExpressionDecimalH\x00R\x07\x64\x65\x63imal\x12J\n\x06struct\x18\x19 \x01(\x0b\x32\x30.substrait.DerivationExpression.ExpressionStructH\x00R\x06struct\x12\x44\n\x04list\x18\x1b \x01(\x0b\x32..substrait.DerivationExpression.ExpressionListH\x00R\x04list\x12\x41\n\x03map\x18\x1c \x01(\x0b\x32-.substrait.DerivationExpression.ExpressionMapH\x00R\x03map\x12Z\n\x0cuser_defined\x18\x1e \x01(\x0b\x32\x35.substrait.DerivationExpression.ExpressionUserDefinedH\x00R\x0buserDefined\x12\x36\n\x14user_defined_pointer\x18\x1f \x01(\rB\x02\x18\x01H\x00R\x12userDefinedPointer\x12\x30\n\x13type_parameter_name\x18! \x01(\tH\x00R\x11typeParameterName\x12\x36\n\x16integer_parameter_name\x18\" \x01(\tH\x00R\x14integerParameterName\x12)\n\x0finteger_literal\x18# \x01(\x05H\x00R\x0eintegerLiteral\x12\x44\n\x08unary_op\x18$ \x01(\x0b\x32\'.substrait.DerivationExpression.UnaryOpH\x00R\x07unaryOp\x12G\n\tbinary_op\x18% \x01(\x0b\x32(.substrait.DerivationExpression.BinaryOpH\x00R\x08\x62inaryOp\x12\x41\n\x07if_else\x18& \x01(\x0b\x32&.substrait.DerivationExpression.IfElseH\x00R\x06ifElse\x12V\n\x0ereturn_program\x18\' \x01(\x0b\x32-.substrait.DerivationExpression.ReturnProgramH\x00R\rreturnProgram\x1a\xba\x01\n\x13\x45xpressionFixedChar\x12\x37\n\x06length\x18\x01 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x06length\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xb8\x01\n\x11\x45xpressionVarChar\x12\x37\n\x06length\x18\x01 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x06length\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xbc\x01\n\x15\x45xpressionFixedBinary\x12\x37\n\x06length\x18\x01 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x06length\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xf5\x01\n\x11\x45xpressionDecimal\x12\x35\n\x05scale\x18\x01 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x05scale\x12=\n\tprecision\x18\x02 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\tprecision\x12+\n\x11variation_pointer\x18\x03 \x01(\rR\x10variationPointer\x12=\n\x0bnullability\x18\x04 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xb5\x01\n\x10\x45xpressionStruct\x12\x35\n\x05types\x18\x01 \x03(\x0b\x32\x1f.substrait.DerivationExpressionR\x05types\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1aw\n\x15\x45xpressionNamedStruct\x12\x14\n\x05names\x18\x01 \x03(\tR\x05names\x12H\n\x06struct\x18\x02 \x01(\x0b\x32\x30.substrait.DerivationExpression.ExpressionStructR\x06struct\x1a\xb1\x01\n\x0e\x45xpressionList\x12\x33\n\x04type\x18\x01 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x04type\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xe5\x01\n\rExpressionMap\x12\x31\n\x03key\x18\x01 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x03key\x12\x35\n\x05value\x18\x02 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x05value\x12+\n\x11variation_pointer\x18\x03 \x01(\rR\x10variationPointer\x12=\n\x0bnullability\x18\x04 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xa6\x01\n\x15\x45xpressionUserDefined\x12!\n\x0ctype_pointer\x18\x01 \x01(\rR\x0btypePointer\x12+\n\x11variation_pointer\x18\x02 \x01(\rR\x10variationPointer\x12=\n\x0bnullability\x18\x03 \x01(\x0e\x32\x1b.substrait.Type.NullabilityR\x0bnullability\x1a\xcc\x01\n\x06IfElse\x12\x42\n\x0cif_condition\x18\x01 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x0bifCondition\x12<\n\tif_return\x18\x02 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x08ifReturn\x12@\n\x0b\x65lse_return\x18\x03 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\nelseReturn\x1a\xd7\x01\n\x07UnaryOp\x12L\n\x07op_type\x18\x01 \x01(\x0e\x32\x33.substrait.DerivationExpression.UnaryOp.UnaryOpTypeR\x06opType\x12\x31\n\x03\x61rg\x18\x02 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x03\x61rg\"K\n\x0bUnaryOpType\x12\x1d\n\x19UNARY_OP_TYPE_UNSPECIFIED\x10\x00\x12\x1d\n\x19UNARY_OP_TYPE_BOOLEAN_NOT\x10\x01\x1a\xb4\x04\n\x08\x42inaryOp\x12N\n\x07op_type\x18\x01 \x01(\x0e\x32\x35.substrait.DerivationExpression.BinaryOp.BinaryOpTypeR\x06opType\x12\x33\n\x04\x61rg1\x18\x02 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x04\x61rg1\x12\x33\n\x04\x61rg2\x18\x03 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x04\x61rg2\"\xed\x02\n\x0c\x42inaryOpType\x12\x1e\n\x1a\x42INARY_OP_TYPE_UNSPECIFIED\x10\x00\x12\x17\n\x13\x42INARY_OP_TYPE_PLUS\x10\x01\x12\x18\n\x14\x42INARY_OP_TYPE_MINUS\x10\x02\x12\x1b\n\x17\x42INARY_OP_TYPE_MULTIPLY\x10\x03\x12\x19\n\x15\x42INARY_OP_TYPE_DIVIDE\x10\x04\x12\x16\n\x12\x42INARY_OP_TYPE_MIN\x10\x05\x12\x16\n\x12\x42INARY_OP_TYPE_MAX\x10\x06\x12\x1f\n\x1b\x42INARY_OP_TYPE_GREATER_THAN\x10\x07\x12\x1c\n\x18\x42INARY_OP_TYPE_LESS_THAN\x10\x08\x12\x16\n\x12\x42INARY_OP_TYPE_AND\x10\t\x12\x15\n\x11\x42INARY_OP_TYPE_OR\x10\n\x12\x19\n\x15\x42INARY_OP_TYPE_EQUALS\x10\x0b\x12\x19\n\x15\x42INARY_OP_TYPE_COVERS\x10\x0c\x1a\x9a\x02\n\rReturnProgram\x12Z\n\x0b\x61ssignments\x18\x01 \x03(\x0b\x32\x38.substrait.DerivationExpression.ReturnProgram.AssignmentR\x0b\x61ssignments\x12J\n\x10\x66inal_expression\x18\x02 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\x0f\x66inalExpression\x1a\x61\n\nAssignment\x12\x12\n\x04name\x18\x01 \x01(\tR\x04name\x12?\n\nexpression\x18\x02 \x01(\x0b\x32\x1f.substrait.DerivationExpressionR\nexpressionB\x06\n\x04kindB\x95\x01\n\rcom.substraitB\x14TypeExpressionsProtoP\x01Z*github.com/substrait-io/substrait-go/proto\xa2\x02\x03SXX\xaa\x02\tSubstrait\xca\x02\tSubstrait\xe2\x02\x15Substrait\\GPBMetadata\xea\x02\tSubstraitb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'substrait.type_expressions_pb2', _globals)
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  DESCRIPTOR._serialized_options = b'\n\rcom.substraitB\024TypeExpressionsProtoP\001Z*github.com/substrait-io/substrait-go/proto\242\002\003SXX\252\002\tSubstrait\312\002\tSubstrait\342\002\025Substrait\\GPBMetadata\352\002\tSubstrait'
  _DERIVATIONEXPRESSION.fields_by_name['user_defined_pointer']._options = None
  _DERIVATIONEXPRESSION.fields_by_name['user_defined_pointer']._serialized_options = b'\030\001'
  _globals['_DERIVATIONEXPRESSION']._serialized_start=70
  _globals['_DERIVATIONEXPRESSION']._serialized_end=5010
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONFIXEDCHAR']._serialized_start=2027
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONFIXEDCHAR']._serialized_end=2213
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONVARCHAR']._serialized_start=2216
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONVARCHAR']._serialized_end=2400
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONFIXEDBINARY']._serialized_start=2403
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONFIXEDBINARY']._serialized_end=2591
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONDECIMAL']._serialized_start=2594
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONDECIMAL']._serialized_end=2839
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONSTRUCT']._serialized_start=2842
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONSTRUCT']._serialized_end=3023
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONNAMEDSTRUCT']._serialized_start=3025
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONNAMEDSTRUCT']._serialized_end=3144
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONLIST']._serialized_start=3147
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONLIST']._serialized_end=3324
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONMAP']._serialized_start=3327
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONMAP']._serialized_end=3556
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONUSERDEFINED']._serialized_start=3559
  _globals['_DERIVATIONEXPRESSION_EXPRESSIONUSERDEFINED']._serialized_end=3725
  _globals['_DERIVATIONEXPRESSION_IFELSE']._serialized_start=3728
  _globals['_DERIVATIONEXPRESSION_IFELSE']._serialized_end=3932
  _globals['_DERIVATIONEXPRESSION_UNARYOP']._serialized_start=3935
  _globals['_DERIVATIONEXPRESSION_UNARYOP']._serialized_end=4150
  _globals['_DERIVATIONEXPRESSION_UNARYOP_UNARYOPTYPE']._serialized_start=4075
  _globals['_DERIVATIONEXPRESSION_UNARYOP_UNARYOPTYPE']._serialized_end=4150
  _globals['_DERIVATIONEXPRESSION_BINARYOP']._serialized_start=4153
  _globals['_DERIVATIONEXPRESSION_BINARYOP']._serialized_end=4717
  _globals['_DERIVATIONEXPRESSION_BINARYOP_BINARYOPTYPE']._serialized_start=4352
  _globals['_DERIVATIONEXPRESSION_BINARYOP_BINARYOPTYPE']._serialized_end=4717
  _globals['_DERIVATIONEXPRESSION_RETURNPROGRAM']._serialized_start=4720
  _globals['_DERIVATIONEXPRESSION_RETURNPROGRAM']._serialized_end=5002
  _globals['_DERIVATIONEXPRESSION_RETURNPROGRAM_ASSIGNMENT']._serialized_start=4905
  _globals['_DERIVATIONEXPRESSION_RETURNPROGRAM_ASSIGNMENT']._serialized_end=5002
# @@protoc_insertion_point(module_scope)