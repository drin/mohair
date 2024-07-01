// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: substrait/capabilities.proto
// Protobuf C++ Version: 5.27.1

#include "../substrait/capabilities.pb.h"

#include <algorithm>
#include <type_traits>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/generated_message_tctable_impl.h"
#include "google/protobuf/extension_set.h"
#include "google/protobuf/wire_format_lite.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/reflection_ops.h"
#include "google/protobuf/wire_format.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"
PROTOBUF_PRAGMA_INIT_SEG
namespace _pb = ::google::protobuf;
namespace _pbi = ::google::protobuf::internal;
namespace _fl = ::google::protobuf::internal::field_layout;
namespace substrait {

inline constexpr Capabilities_SimpleExtension::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : function_keys_{},
        type_keys_{},
        type_variation_keys_{},
        uri_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR Capabilities_SimpleExtension::Capabilities_SimpleExtension(::_pbi::ConstantInitialized)
    : _impl_(::_pbi::ConstantInitialized()) {}
struct Capabilities_SimpleExtensionDefaultTypeInternal {
  PROTOBUF_CONSTEXPR Capabilities_SimpleExtensionDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~Capabilities_SimpleExtensionDefaultTypeInternal() {}
  union {
    Capabilities_SimpleExtension _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 Capabilities_SimpleExtensionDefaultTypeInternal _Capabilities_SimpleExtension_default_instance_;

inline constexpr Capabilities::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : substrait_versions_{},
        advanced_extension_type_urls_{},
        simple_extensions_{},
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR Capabilities::Capabilities(::_pbi::ConstantInitialized)
    : _impl_(::_pbi::ConstantInitialized()) {}
struct CapabilitiesDefaultTypeInternal {
  PROTOBUF_CONSTEXPR CapabilitiesDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~CapabilitiesDefaultTypeInternal() {}
  union {
    Capabilities _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 CapabilitiesDefaultTypeInternal _Capabilities_default_instance_;
}  // namespace substrait
static constexpr const ::_pb::EnumDescriptor**
    file_level_enum_descriptors_substrait_2fcapabilities_2eproto = nullptr;
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_substrait_2fcapabilities_2eproto = nullptr;
const ::uint32_t
    TableStruct_substrait_2fcapabilities_2eproto::offsets[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
        protodesc_cold) = {
        ~0u,  // no _has_bits_
        PROTOBUF_FIELD_OFFSET(::substrait::Capabilities_SimpleExtension, _internal_metadata_),
        ~0u,  // no _extensions_
        ~0u,  // no _oneof_case_
        ~0u,  // no _weak_field_map_
        ~0u,  // no _inlined_string_donated_
        ~0u,  // no _split_
        ~0u,  // no sizeof(Split)
        PROTOBUF_FIELD_OFFSET(::substrait::Capabilities_SimpleExtension, _impl_.uri_),
        PROTOBUF_FIELD_OFFSET(::substrait::Capabilities_SimpleExtension, _impl_.function_keys_),
        PROTOBUF_FIELD_OFFSET(::substrait::Capabilities_SimpleExtension, _impl_.type_keys_),
        PROTOBUF_FIELD_OFFSET(::substrait::Capabilities_SimpleExtension, _impl_.type_variation_keys_),
        ~0u,  // no _has_bits_
        PROTOBUF_FIELD_OFFSET(::substrait::Capabilities, _internal_metadata_),
        ~0u,  // no _extensions_
        ~0u,  // no _oneof_case_
        ~0u,  // no _weak_field_map_
        ~0u,  // no _inlined_string_donated_
        ~0u,  // no _split_
        ~0u,  // no sizeof(Split)
        PROTOBUF_FIELD_OFFSET(::substrait::Capabilities, _impl_.substrait_versions_),
        PROTOBUF_FIELD_OFFSET(::substrait::Capabilities, _impl_.advanced_extension_type_urls_),
        PROTOBUF_FIELD_OFFSET(::substrait::Capabilities, _impl_.simple_extensions_),
};

static const ::_pbi::MigrationSchema
    schemas[] ABSL_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
        {0, -1, -1, sizeof(::substrait::Capabilities_SimpleExtension)},
        {12, -1, -1, sizeof(::substrait::Capabilities)},
};
static const ::_pb::Message* const file_default_instances[] = {
    &::substrait::_Capabilities_SimpleExtension_default_instance_._instance,
    &::substrait::_Capabilities_default_instance_._instance,
};
const char descriptor_table_protodef_substrait_2fcapabilities_2eproto[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
    protodesc_cold) = {
    "\n\034substrait/capabilities.proto\022\tsubstrai"
    "t\"\354\002\n\014Capabilities\022-\n\022substrait_versions"
    "\030\001 \003(\tR\021substraitVersions\022\?\n\034advanced_ex"
    "tension_type_urls\030\002 \003(\tR\031advancedExtensi"
    "onTypeUrls\022T\n\021simple_extensions\030\003 \003(\0132\'."
    "substrait.Capabilities.SimpleExtensionR\020"
    "simpleExtensions\032\225\001\n\017SimpleExtension\022\020\n\003"
    "uri\030\001 \001(\tR\003uri\022#\n\rfunction_keys\030\002 \003(\tR\014f"
    "unctionKeys\022\033\n\ttype_keys\030\003 \003(\tR\010typeKeys"
    "\022.\n\023type_variation_keys\030\004 \003(\tR\021typeVaria"
    "tionKeysB\222\001\n\rcom.substraitB\021Capabilities"
    "ProtoP\001Z*github.com/substrait-io/substra"
    "it-go/proto\242\002\003SXX\252\002\tSubstrait\312\002\tSubstrai"
    "t\342\002\025Substrait\\GPBMetadata\352\002\tSubstraitb\006p"
    "roto3"
};
static ::absl::once_flag descriptor_table_substrait_2fcapabilities_2eproto_once;
PROTOBUF_CONSTINIT const ::_pbi::DescriptorTable descriptor_table_substrait_2fcapabilities_2eproto = {
    false,
    false,
    565,
    descriptor_table_protodef_substrait_2fcapabilities_2eproto,
    "substrait/capabilities.proto",
    &descriptor_table_substrait_2fcapabilities_2eproto_once,
    nullptr,
    0,
    2,
    schemas,
    file_default_instances,
    TableStruct_substrait_2fcapabilities_2eproto::offsets,
    file_level_enum_descriptors_substrait_2fcapabilities_2eproto,
    file_level_service_descriptors_substrait_2fcapabilities_2eproto,
};
namespace substrait {
// ===================================================================

class Capabilities_SimpleExtension::_Internal {
 public:
};

Capabilities_SimpleExtension::Capabilities_SimpleExtension(::google::protobuf::Arena* arena)
    : ::google::protobuf::Message(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:substrait.Capabilities.SimpleExtension)
}
inline PROTOBUF_NDEBUG_INLINE Capabilities_SimpleExtension::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from, const ::substrait::Capabilities_SimpleExtension& from_msg)
      : function_keys_{visibility, arena, from.function_keys_},
        type_keys_{visibility, arena, from.type_keys_},
        type_variation_keys_{visibility, arena, from.type_variation_keys_},
        uri_(arena, from.uri_),
        _cached_size_{0} {}

Capabilities_SimpleExtension::Capabilities_SimpleExtension(
    ::google::protobuf::Arena* arena,
    const Capabilities_SimpleExtension& from)
    : ::google::protobuf::Message(arena) {
  Capabilities_SimpleExtension* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_, from);

  // @@protoc_insertion_point(copy_constructor:substrait.Capabilities.SimpleExtension)
}
inline PROTOBUF_NDEBUG_INLINE Capabilities_SimpleExtension::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : function_keys_{visibility, arena},
        type_keys_{visibility, arena},
        type_variation_keys_{visibility, arena},
        uri_(arena),
        _cached_size_{0} {}

inline void Capabilities_SimpleExtension::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
}
Capabilities_SimpleExtension::~Capabilities_SimpleExtension() {
  // @@protoc_insertion_point(destructor:substrait.Capabilities.SimpleExtension)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void Capabilities_SimpleExtension::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.uri_.Destroy();
  _impl_.~Impl_();
}

const ::google::protobuf::MessageLite::ClassData*
Capabilities_SimpleExtension::GetClassData() const {
  PROTOBUF_CONSTINIT static const ::google::protobuf::MessageLite::
      ClassDataFull _data_ = {
          {
              &_table_.header,
              nullptr,  // OnDemandRegisterArenaDtor
              nullptr,  // IsInitialized
              PROTOBUF_FIELD_OFFSET(Capabilities_SimpleExtension, _impl_._cached_size_),
              false,
          },
          &Capabilities_SimpleExtension::MergeImpl,
          &Capabilities_SimpleExtension::kDescriptorMethods,
          &descriptor_table_substrait_2fcapabilities_2eproto,
          nullptr,  // tracker
      };
  ::google::protobuf::internal::PrefetchToLocalCache(&_data_);
  ::google::protobuf::internal::PrefetchToLocalCache(_data_.tc_table);
  return _data_.base();
}
PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<2, 4, 0, 91, 2> Capabilities_SimpleExtension::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    4, 24,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967280,  // skipmap
    offsetof(decltype(_table_), field_entries),
    4,  // num_field_entries
    0,  // num_aux_entries
    offsetof(decltype(_table_), field_names),  // no aux_entries
    &_Capabilities_SimpleExtension_default_instance_._instance,
    nullptr,  // post_loop_handler
    ::_pbi::TcParser::GenericFallback,  // fallback
    #ifdef PROTOBUF_PREFETCH_PARSE_TABLE
    ::_pbi::TcParser::GetTable<::substrait::Capabilities_SimpleExtension>(),  // to_prefetch
    #endif  // PROTOBUF_PREFETCH_PARSE_TABLE
  }, {{
    // repeated string type_variation_keys = 4 [json_name = "typeVariationKeys"];
    {::_pbi::TcParser::FastUR1,
     {34, 63, 0, PROTOBUF_FIELD_OFFSET(Capabilities_SimpleExtension, _impl_.type_variation_keys_)}},
    // string uri = 1 [json_name = "uri"];
    {::_pbi::TcParser::FastUS1,
     {10, 63, 0, PROTOBUF_FIELD_OFFSET(Capabilities_SimpleExtension, _impl_.uri_)}},
    // repeated string function_keys = 2 [json_name = "functionKeys"];
    {::_pbi::TcParser::FastUR1,
     {18, 63, 0, PROTOBUF_FIELD_OFFSET(Capabilities_SimpleExtension, _impl_.function_keys_)}},
    // repeated string type_keys = 3 [json_name = "typeKeys"];
    {::_pbi::TcParser::FastUR1,
     {26, 63, 0, PROTOBUF_FIELD_OFFSET(Capabilities_SimpleExtension, _impl_.type_keys_)}},
  }}, {{
    65535, 65535
  }}, {{
    // string uri = 1 [json_name = "uri"];
    {PROTOBUF_FIELD_OFFSET(Capabilities_SimpleExtension, _impl_.uri_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kUtf8String | ::_fl::kRepAString)},
    // repeated string function_keys = 2 [json_name = "functionKeys"];
    {PROTOBUF_FIELD_OFFSET(Capabilities_SimpleExtension, _impl_.function_keys_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kUtf8String | ::_fl::kRepSString)},
    // repeated string type_keys = 3 [json_name = "typeKeys"];
    {PROTOBUF_FIELD_OFFSET(Capabilities_SimpleExtension, _impl_.type_keys_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kUtf8String | ::_fl::kRepSString)},
    // repeated string type_variation_keys = 4 [json_name = "typeVariationKeys"];
    {PROTOBUF_FIELD_OFFSET(Capabilities_SimpleExtension, _impl_.type_variation_keys_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kUtf8String | ::_fl::kRepSString)},
  }},
  // no aux_entries
  {{
    "\46\3\15\11\23\0\0\0"
    "substrait.Capabilities.SimpleExtension"
    "uri"
    "function_keys"
    "type_keys"
    "type_variation_keys"
  }},
};

PROTOBUF_NOINLINE void Capabilities_SimpleExtension::Clear() {
// @@protoc_insertion_point(message_clear_start:substrait.Capabilities.SimpleExtension)
  ::google::protobuf::internal::TSanWrite(&_impl_);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.function_keys_.Clear();
  _impl_.type_keys_.Clear();
  _impl_.type_variation_keys_.Clear();
  _impl_.uri_.ClearToEmpty();
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

::uint8_t* Capabilities_SimpleExtension::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:substrait.Capabilities.SimpleExtension)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // string uri = 1 [json_name = "uri"];
  if (!this->_internal_uri().empty()) {
    const std::string& _s = this->_internal_uri();
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        _s.data(), static_cast<int>(_s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "substrait.Capabilities.SimpleExtension.uri");
    target = stream->WriteStringMaybeAliased(1, _s, target);
  }

  // repeated string function_keys = 2 [json_name = "functionKeys"];
  for (int i = 0, n = this->_internal_function_keys_size(); i < n; ++i) {
    const auto& s = this->_internal_function_keys().Get(i);
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        s.data(), static_cast<int>(s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "substrait.Capabilities.SimpleExtension.function_keys");
    target = stream->WriteString(2, s, target);
  }

  // repeated string type_keys = 3 [json_name = "typeKeys"];
  for (int i = 0, n = this->_internal_type_keys_size(); i < n; ++i) {
    const auto& s = this->_internal_type_keys().Get(i);
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        s.data(), static_cast<int>(s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "substrait.Capabilities.SimpleExtension.type_keys");
    target = stream->WriteString(3, s, target);
  }

  // repeated string type_variation_keys = 4 [json_name = "typeVariationKeys"];
  for (int i = 0, n = this->_internal_type_variation_keys_size(); i < n; ++i) {
    const auto& s = this->_internal_type_variation_keys().Get(i);
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        s.data(), static_cast<int>(s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "substrait.Capabilities.SimpleExtension.type_variation_keys");
    target = stream->WriteString(4, s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target =
        ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:substrait.Capabilities.SimpleExtension)
  return target;
}

::size_t Capabilities_SimpleExtension::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:substrait.Capabilities.SimpleExtension)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  ::_pbi::Prefetch5LinesFrom7Lines(reinterpret_cast<const void*>(this));
  // repeated string function_keys = 2 [json_name = "functionKeys"];
  total_size += 1 * ::google::protobuf::internal::FromIntSize(_internal_function_keys().size());
  for (int i = 0, n = _internal_function_keys().size(); i < n; ++i) {
    total_size += ::google::protobuf::internal::WireFormatLite::StringSize(
        _internal_function_keys().Get(i));
  }
  // repeated string type_keys = 3 [json_name = "typeKeys"];
  total_size += 1 * ::google::protobuf::internal::FromIntSize(_internal_type_keys().size());
  for (int i = 0, n = _internal_type_keys().size(); i < n; ++i) {
    total_size += ::google::protobuf::internal::WireFormatLite::StringSize(
        _internal_type_keys().Get(i));
  }
  // repeated string type_variation_keys = 4 [json_name = "typeVariationKeys"];
  total_size += 1 * ::google::protobuf::internal::FromIntSize(_internal_type_variation_keys().size());
  for (int i = 0, n = _internal_type_variation_keys().size(); i < n; ++i) {
    total_size += ::google::protobuf::internal::WireFormatLite::StringSize(
        _internal_type_variation_keys().Get(i));
  }
  // string uri = 1 [json_name = "uri"];
  if (!this->_internal_uri().empty()) {
    total_size += 1 + ::google::protobuf::internal::WireFormatLite::StringSize(
                                    this->_internal_uri());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}


void Capabilities_SimpleExtension::MergeImpl(::google::protobuf::MessageLite& to_msg, const ::google::protobuf::MessageLite& from_msg) {
  auto* const _this = static_cast<Capabilities_SimpleExtension*>(&to_msg);
  auto& from = static_cast<const Capabilities_SimpleExtension&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:substrait.Capabilities.SimpleExtension)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_internal_mutable_function_keys()->MergeFrom(from._internal_function_keys());
  _this->_internal_mutable_type_keys()->MergeFrom(from._internal_type_keys());
  _this->_internal_mutable_type_variation_keys()->MergeFrom(from._internal_type_variation_keys());
  if (!from._internal_uri().empty()) {
    _this->_internal_set_uri(from._internal_uri());
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void Capabilities_SimpleExtension::CopyFrom(const Capabilities_SimpleExtension& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:substrait.Capabilities.SimpleExtension)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}


void Capabilities_SimpleExtension::InternalSwap(Capabilities_SimpleExtension* PROTOBUF_RESTRICT other) {
  using std::swap;
  auto* arena = GetArena();
  ABSL_DCHECK_EQ(arena, other->GetArena());
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.function_keys_.InternalSwap(&other->_impl_.function_keys_);
  _impl_.type_keys_.InternalSwap(&other->_impl_.type_keys_);
  _impl_.type_variation_keys_.InternalSwap(&other->_impl_.type_variation_keys_);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.uri_, &other->_impl_.uri_, arena);
}

::google::protobuf::Metadata Capabilities_SimpleExtension::GetMetadata() const {
  return ::google::protobuf::Message::GetMetadataImpl(GetClassData()->full());
}
// ===================================================================

class Capabilities::_Internal {
 public:
};

Capabilities::Capabilities(::google::protobuf::Arena* arena)
    : ::google::protobuf::Message(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:substrait.Capabilities)
}
inline PROTOBUF_NDEBUG_INLINE Capabilities::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from, const ::substrait::Capabilities& from_msg)
      : substrait_versions_{visibility, arena, from.substrait_versions_},
        advanced_extension_type_urls_{visibility, arena, from.advanced_extension_type_urls_},
        simple_extensions_{visibility, arena, from.simple_extensions_},
        _cached_size_{0} {}

Capabilities::Capabilities(
    ::google::protobuf::Arena* arena,
    const Capabilities& from)
    : ::google::protobuf::Message(arena) {
  Capabilities* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_, from);

  // @@protoc_insertion_point(copy_constructor:substrait.Capabilities)
}
inline PROTOBUF_NDEBUG_INLINE Capabilities::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : substrait_versions_{visibility, arena},
        advanced_extension_type_urls_{visibility, arena},
        simple_extensions_{visibility, arena},
        _cached_size_{0} {}

inline void Capabilities::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
}
Capabilities::~Capabilities() {
  // @@protoc_insertion_point(destructor:substrait.Capabilities)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void Capabilities::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.~Impl_();
}

const ::google::protobuf::MessageLite::ClassData*
Capabilities::GetClassData() const {
  PROTOBUF_CONSTINIT static const ::google::protobuf::MessageLite::
      ClassDataFull _data_ = {
          {
              &_table_.header,
              nullptr,  // OnDemandRegisterArenaDtor
              nullptr,  // IsInitialized
              PROTOBUF_FIELD_OFFSET(Capabilities, _impl_._cached_size_),
              false,
          },
          &Capabilities::MergeImpl,
          &Capabilities::kDescriptorMethods,
          &descriptor_table_substrait_2fcapabilities_2eproto,
          nullptr,  // tracker
      };
  ::google::protobuf::internal::PrefetchToLocalCache(&_data_);
  ::google::protobuf::internal::PrefetchToLocalCache(_data_.tc_table);
  return _data_.base();
}
PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<2, 3, 1, 77, 2> Capabilities::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    3, 24,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967288,  // skipmap
    offsetof(decltype(_table_), field_entries),
    3,  // num_field_entries
    1,  // num_aux_entries
    offsetof(decltype(_table_), aux_entries),
    &_Capabilities_default_instance_._instance,
    nullptr,  // post_loop_handler
    ::_pbi::TcParser::GenericFallback,  // fallback
    #ifdef PROTOBUF_PREFETCH_PARSE_TABLE
    ::_pbi::TcParser::GetTable<::substrait::Capabilities>(),  // to_prefetch
    #endif  // PROTOBUF_PREFETCH_PARSE_TABLE
  }, {{
    {::_pbi::TcParser::MiniParse, {}},
    // repeated string substrait_versions = 1 [json_name = "substraitVersions"];
    {::_pbi::TcParser::FastUR1,
     {10, 63, 0, PROTOBUF_FIELD_OFFSET(Capabilities, _impl_.substrait_versions_)}},
    // repeated string advanced_extension_type_urls = 2 [json_name = "advancedExtensionTypeUrls"];
    {::_pbi::TcParser::FastUR1,
     {18, 63, 0, PROTOBUF_FIELD_OFFSET(Capabilities, _impl_.advanced_extension_type_urls_)}},
    // repeated .substrait.Capabilities.SimpleExtension simple_extensions = 3 [json_name = "simpleExtensions"];
    {::_pbi::TcParser::FastMtR1,
     {26, 63, 0, PROTOBUF_FIELD_OFFSET(Capabilities, _impl_.simple_extensions_)}},
  }}, {{
    65535, 65535
  }}, {{
    // repeated string substrait_versions = 1 [json_name = "substraitVersions"];
    {PROTOBUF_FIELD_OFFSET(Capabilities, _impl_.substrait_versions_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kUtf8String | ::_fl::kRepSString)},
    // repeated string advanced_extension_type_urls = 2 [json_name = "advancedExtensionTypeUrls"];
    {PROTOBUF_FIELD_OFFSET(Capabilities, _impl_.advanced_extension_type_urls_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kUtf8String | ::_fl::kRepSString)},
    // repeated .substrait.Capabilities.SimpleExtension simple_extensions = 3 [json_name = "simpleExtensions"];
    {PROTOBUF_FIELD_OFFSET(Capabilities, _impl_.simple_extensions_), 0, 0,
    (0 | ::_fl::kFcRepeated | ::_fl::kMessage | ::_fl::kTvTable)},
  }}, {{
    {::_pbi::TcParser::GetTable<::substrait::Capabilities_SimpleExtension>()},
  }}, {{
    "\26\22\34\0\0\0\0\0"
    "substrait.Capabilities"
    "substrait_versions"
    "advanced_extension_type_urls"
  }},
};

PROTOBUF_NOINLINE void Capabilities::Clear() {
// @@protoc_insertion_point(message_clear_start:substrait.Capabilities)
  ::google::protobuf::internal::TSanWrite(&_impl_);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.substrait_versions_.Clear();
  _impl_.advanced_extension_type_urls_.Clear();
  _impl_.simple_extensions_.Clear();
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

::uint8_t* Capabilities::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:substrait.Capabilities)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // repeated string substrait_versions = 1 [json_name = "substraitVersions"];
  for (int i = 0, n = this->_internal_substrait_versions_size(); i < n; ++i) {
    const auto& s = this->_internal_substrait_versions().Get(i);
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        s.data(), static_cast<int>(s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "substrait.Capabilities.substrait_versions");
    target = stream->WriteString(1, s, target);
  }

  // repeated string advanced_extension_type_urls = 2 [json_name = "advancedExtensionTypeUrls"];
  for (int i = 0, n = this->_internal_advanced_extension_type_urls_size(); i < n; ++i) {
    const auto& s = this->_internal_advanced_extension_type_urls().Get(i);
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        s.data(), static_cast<int>(s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "substrait.Capabilities.advanced_extension_type_urls");
    target = stream->WriteString(2, s, target);
  }

  // repeated .substrait.Capabilities.SimpleExtension simple_extensions = 3 [json_name = "simpleExtensions"];
  for (unsigned i = 0, n = static_cast<unsigned>(
                           this->_internal_simple_extensions_size());
       i < n; i++) {
    const auto& repfield = this->_internal_simple_extensions().Get(i);
    target =
        ::google::protobuf::internal::WireFormatLite::InternalWriteMessage(
            3, repfield, repfield.GetCachedSize(),
            target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target =
        ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:substrait.Capabilities)
  return target;
}

::size_t Capabilities::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:substrait.Capabilities)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  ::_pbi::Prefetch5LinesFrom7Lines(reinterpret_cast<const void*>(this));
  // repeated string substrait_versions = 1 [json_name = "substraitVersions"];
  total_size += 1 * ::google::protobuf::internal::FromIntSize(_internal_substrait_versions().size());
  for (int i = 0, n = _internal_substrait_versions().size(); i < n; ++i) {
    total_size += ::google::protobuf::internal::WireFormatLite::StringSize(
        _internal_substrait_versions().Get(i));
  }
  // repeated string advanced_extension_type_urls = 2 [json_name = "advancedExtensionTypeUrls"];
  total_size += 1 * ::google::protobuf::internal::FromIntSize(_internal_advanced_extension_type_urls().size());
  for (int i = 0, n = _internal_advanced_extension_type_urls().size(); i < n; ++i) {
    total_size += ::google::protobuf::internal::WireFormatLite::StringSize(
        _internal_advanced_extension_type_urls().Get(i));
  }
  // repeated .substrait.Capabilities.SimpleExtension simple_extensions = 3 [json_name = "simpleExtensions"];
  total_size += 1UL * this->_internal_simple_extensions_size();
  for (const auto& msg : this->_internal_simple_extensions()) {
    total_size += ::google::protobuf::internal::WireFormatLite::MessageSize(msg);
  }
  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}


void Capabilities::MergeImpl(::google::protobuf::MessageLite& to_msg, const ::google::protobuf::MessageLite& from_msg) {
  auto* const _this = static_cast<Capabilities*>(&to_msg);
  auto& from = static_cast<const Capabilities&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:substrait.Capabilities)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_internal_mutable_substrait_versions()->MergeFrom(from._internal_substrait_versions());
  _this->_internal_mutable_advanced_extension_type_urls()->MergeFrom(from._internal_advanced_extension_type_urls());
  _this->_internal_mutable_simple_extensions()->MergeFrom(
      from._internal_simple_extensions());
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void Capabilities::CopyFrom(const Capabilities& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:substrait.Capabilities)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}


void Capabilities::InternalSwap(Capabilities* PROTOBUF_RESTRICT other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.substrait_versions_.InternalSwap(&other->_impl_.substrait_versions_);
  _impl_.advanced_extension_type_urls_.InternalSwap(&other->_impl_.advanced_extension_type_urls_);
  _impl_.simple_extensions_.InternalSwap(&other->_impl_.simple_extensions_);
}

::google::protobuf::Metadata Capabilities::GetMetadata() const {
  return ::google::protobuf::Message::GetMetadataImpl(GetClassData()->full());
}
// @@protoc_insertion_point(namespace_scope)
}  // namespace substrait
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::std::false_type
    _static_init2_ PROTOBUF_UNUSED =
        (::_pbi::AddDescriptors(&descriptor_table_substrait_2fcapabilities_2eproto),
         ::std::false_type{});
#include "google/protobuf/port_undef.inc"
