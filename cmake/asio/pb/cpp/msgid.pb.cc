// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: msgid.proto
// Protobuf C++ Version: 5.28.0

#include "msgid.pb.h"

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
static const ::_pb::EnumDescriptor* file_level_enum_descriptors_msgid_2eproto[1];
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_msgid_2eproto = nullptr;
const ::uint32_t TableStruct_msgid_2eproto::offsets[1] = {};
static constexpr ::_pbi::MigrationSchema* schemas = nullptr;
static constexpr ::_pb::Message* const* file_default_instances = nullptr;
const char descriptor_table_protodef_msgid_2eproto[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
    protodesc_cold) = {
    "\n\013msgid.proto\022\006netmsg*q\n\010NetCmdID\022\013\n\007Cmd"
    "None\020\000\022\010\n\004Ping\020\001\022\010\n\004Pong\020\002\022\020\n\014SecurityIn"
    "it\020\003\022\n\n\006ReqReg\020Q\022\n\n\006RspReg\020R\022\014\n\010ReqLogin"
    "\020S\022\014\n\010RspLogin\020Tb\006proto3"
};
static ::absl::once_flag descriptor_table_msgid_2eproto_once;
PROTOBUF_CONSTINIT const ::_pbi::DescriptorTable descriptor_table_msgid_2eproto = {
    false,
    false,
    144,
    descriptor_table_protodef_msgid_2eproto,
    "msgid.proto",
    &descriptor_table_msgid_2eproto_once,
    nullptr,
    0,
    0,
    schemas,
    file_default_instances,
    TableStruct_msgid_2eproto::offsets,
    file_level_enum_descriptors_msgid_2eproto,
    file_level_service_descriptors_msgid_2eproto,
};
namespace netmsg {
const ::google::protobuf::EnumDescriptor* NetCmdID_descriptor() {
  ::google::protobuf::internal::AssignDescriptors(&descriptor_table_msgid_2eproto);
  return file_level_enum_descriptors_msgid_2eproto[0];
}
PROTOBUF_CONSTINIT const uint32_t NetCmdID_internal_data_[] = {
    262144u, 96u, 0u, 0u, 122880u, };
bool NetCmdID_IsValid(int value) {
  return ::_pbi::ValidateEnum(value, NetCmdID_internal_data_);
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
        (::_pbi::AddDescriptors(&descriptor_table_msgid_2eproto),
         ::std::false_type{});
#include "google/protobuf/port_undef.inc"
