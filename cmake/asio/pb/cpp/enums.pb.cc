// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: enums.proto
// Protobuf C++ Version: 5.28.3

#include "enums.pb.h"

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
namespace netmsg {
}  // namespace netmsg
static const ::_pb::EnumDescriptor* file_level_enum_descriptors_enums_2eproto[4];
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_enums_2eproto = nullptr;
const ::uint32_t TableStruct_enums_2eproto::offsets[1] = {};
static constexpr ::_pbi::MigrationSchema* schemas = nullptr;
static constexpr ::_pb::Message* const* file_default_instances = nullptr;
const char descriptor_table_protodef_enums_2eproto[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
    protodesc_cold) = {
    "\n\013enums.proto\022\006netmsg*\356\001\n\016EUserErrorType"
    "\022\016\n\nUserETNone\020\000\022\014\n\010UserETOK\020\001\022\023\n\017UserET"
    "NameExist\020\002\022\030\n\024UserETNameOrPwdError\020\003\022\026\n"
    "\022UserETNameNotExist\020\004\022\023\n\017UserETRegFailed"
    "\020\005\022\026\n\022UserETTypeNotExist\020\006\022\027\n\023UserETReco"
    "nnectFail\020\007\022\025\n\021UserETReconnectOK\020\010\022\032\n\026Us"
    "erAnotherClientLogin\020\t*F\n\tEUserType\022\023\n\017U"
    "serTypeGeneral\020\000\022\021\n\rUserTypeGuest\020\001\022\021\n\rU"
    "serTypeThird\020\002*(\n\014EChannelType\022\r\n\tCTDefa"
    "ult\020\000\022\t\n\005Third\020\001*\031\n\010ETipType\022\r\n\tTipTypeU"
    "p\020\000b\006proto3"
};
static ::absl::once_flag descriptor_table_enums_2eproto_once;
PROTOBUF_CONSTINIT const ::_pbi::DescriptorTable descriptor_table_enums_2eproto = {
    false,
    false,
    411,
    descriptor_table_protodef_enums_2eproto,
    "enums.proto",
    &descriptor_table_enums_2eproto_once,
    nullptr,
    0,
    0,
    schemas,
    file_default_instances,
    TableStruct_enums_2eproto::offsets,
    file_level_enum_descriptors_enums_2eproto,
    file_level_service_descriptors_enums_2eproto,
};
namespace netmsg {
const ::google::protobuf::EnumDescriptor* EUserErrorType_descriptor() {
  ::google::protobuf::internal::AssignDescriptors(&descriptor_table_enums_2eproto);
  return file_level_enum_descriptors_enums_2eproto[0];
}
PROTOBUF_CONSTINIT const uint32_t EUserErrorType_internal_data_[] = {
    655360u, 0u, };
bool EUserErrorType_IsValid(int value) {
  return 0 <= value && value <= 9;
}
const ::google::protobuf::EnumDescriptor* EUserType_descriptor() {
  ::google::protobuf::internal::AssignDescriptors(&descriptor_table_enums_2eproto);
  return file_level_enum_descriptors_enums_2eproto[1];
}
PROTOBUF_CONSTINIT const uint32_t EUserType_internal_data_[] = {
    196608u, 0u, };
bool EUserType_IsValid(int value) {
  return 0 <= value && value <= 2;
}
const ::google::protobuf::EnumDescriptor* EChannelType_descriptor() {
  ::google::protobuf::internal::AssignDescriptors(&descriptor_table_enums_2eproto);
  return file_level_enum_descriptors_enums_2eproto[2];
}
PROTOBUF_CONSTINIT const uint32_t EChannelType_internal_data_[] = {
    131072u, 0u, };
bool EChannelType_IsValid(int value) {
  return 0 <= value && value <= 1;
}
const ::google::protobuf::EnumDescriptor* ETipType_descriptor() {
  ::google::protobuf::internal::AssignDescriptors(&descriptor_table_enums_2eproto);
  return file_level_enum_descriptors_enums_2eproto[3];
}
PROTOBUF_CONSTINIT const uint32_t ETipType_internal_data_[] = {
    65536u, 0u, };
bool ETipType_IsValid(int value) {
  return 0 <= value && value <= 0;
}
// @@protoc_insertion_point(namespace_scope)
}  // namespace netmsg
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::std::false_type
    _static_init2_ PROTOBUF_UNUSED =
        (::_pbi::AddDescriptors(&descriptor_table_enums_2eproto),
         ::std::false_type{});
#include "google/protobuf/port_undef.inc"
