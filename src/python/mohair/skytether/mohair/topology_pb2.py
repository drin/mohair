# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# NO CHECKED-IN PROTOBUF GENCODE
# source: skytether/mohair/topology.proto
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
    'skytether/mohair/topology.proto'
)
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from google.protobuf import any_pb2 as google_dot_protobuf_dot_any__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x1fskytether/mohair/topology.proto\x12\x10skytether.mohair\x1a\x19google/protobuf/any.proto\"R\n\x10\x43omputeResources\x12\x1f\n\x0b\x63ount_cores\x18\x01 \x01(\rR\ncountCores\x12\x1d\n\ncore_freqs\x18\x02 \x03(\x02R\tcoreFreqs\"\x83\x02\n\x0fMemoryResources\x12\'\n\x0f\x63ount_mebibytes\x18\x01 \x01(\rR\x0e\x63ountMebibytes\x12\x42\n\x0bmemory_type\x18\x02 \x01(\x0e\x32\x1c.skytether.mohair.MemoryTypeH\x00R\nmemoryType\x88\x01\x01\x12*\n\x0e\x63ount_channels\x18\x03 \x01(\rH\x01R\rcountChannels\x88\x01\x01\x12$\n\x0bmemory_freq\x18\x04 \x01(\rH\x02R\nmemoryFreq\x88\x01\x01\x42\x0e\n\x0c_memory_typeB\x11\n\x0f_count_channelsB\x0e\n\x0c_memory_freq\"\x95\x03\n\rServiceConfig\x12\x1b\n\tis_active\x18\x01 \x01(\x08R\x08isActive\x12)\n\x10service_location\x18\x02 \x01(\tR\x0fserviceLocation\x12\x44\n\x0eplatform_class\x18\x03 \x01(\x0e\x32\x1d.skytether.mohair.DeviceClassR\rplatformClass\x12P\n\x13\x64ownstream_services\x18\x04 \x03(\x0b\x32\x1f.skytether.mohair.ServiceConfigR\x12\x64ownstreamServices\x12G\n\x0bservice_mem\x18\x05 \x01(\x0b\x32!.skytether.mohair.MemoryResourcesH\x00R\nserviceMem\x88\x01\x01\x12K\n\x0fservice_compute\x18\x06 \x03(\x0b\x32\".skytether.mohair.ComputeResourcesR\x0eserviceComputeB\x0e\n\x0c_service_mem*\x82\x01\n\x0b\x44\x65viceClass\x12\x17\n\x13\x44\x45VICE_CLASS_SERVER\x10\x00\x12\x16\n\x12\x44\x45VICE_CLASS_DRIVE\x10\x01\x12\x14\n\x10\x44\x45VICE_CLASS_SOC\x10\x02\x12\x16\n\x12\x44\x45VICE_CLASS_ARRAY\x10\x03\x12\x14\n\x10\x44\x45VICE_CLASS_DPU\x10\x04*v\n\nMemoryType\x12\x13\n\x0fMEMORY_TYPE_DDR\x10\x00\x12\x14\n\x10MEMORY_TYPE_GDDR\x10\x01\x12\x13\n\x0fMEMORY_TYPE_NVM\x10\x02\x12\x13\n\x0fMEMORY_TYPE_CXL\x10\x03\x12\x13\n\x0fMEMORY_TYPE_CAM\x10\x04\x42\x86\x01\n\x14\x63om.skytether.mohairB\rTopologyProtoP\x01\xa2\x02\x03SMX\xaa\x02\x10Skytether.Mohair\xca\x02\x10Skytether\\Mohair\xe2\x02\x1cSkytether\\Mohair\\GPBMetadata\xea\x02\x11Skytether::Mohairb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'skytether.mohair.topology_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  _globals['DESCRIPTOR']._loaded_options = None
  _globals['DESCRIPTOR']._serialized_options = b'\n\024com.skytether.mohairB\rTopologyProtoP\001\242\002\003SMX\252\002\020Skytether.Mohair\312\002\020Skytether\\Mohair\342\002\034Skytether\\Mohair\\GPBMetadata\352\002\021Skytether::Mohair'
  _globals['_DEVICECLASS']._serialized_start=835
  _globals['_DEVICECLASS']._serialized_end=965
  _globals['_MEMORYTYPE']._serialized_start=967
  _globals['_MEMORYTYPE']._serialized_end=1085
  _globals['_COMPUTERESOURCES']._serialized_start=80
  _globals['_COMPUTERESOURCES']._serialized_end=162
  _globals['_MEMORYRESOURCES']._serialized_start=165
  _globals['_MEMORYRESOURCES']._serialized_end=424
  _globals['_SERVICECONFIG']._serialized_start=427
  _globals['_SERVICECONFIG']._serialized_end=832
# @@protoc_insertion_point(module_scope)
