// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: msgid.proto
// Protobuf C++ Version: 5.28.3

#ifndef GOOGLE_PROTOBUF_INCLUDED_msgid_2eproto_2epb_2eh
#define GOOGLE_PROTOBUF_INCLUDED_msgid_2eproto_2epb_2eh

#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "google/protobuf/runtime_version.h"
#if PROTOBUF_VERSION != 5028003
#error "Protobuf C++ gencode is built with an incompatible version of"
#error "Protobuf C++ headers/runtime. See"
#error "https://protobuf.dev/support/cross-version-runtime-guarantee/#cpp"
#endif
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/arena.h"
#include "google/protobuf/arenastring.h"
#include "google/protobuf/generated_message_tctable_decl.h"
#include "google/protobuf/generated_message_util.h"
#include "google/protobuf/metadata_lite.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/repeated_field.h"  // IWYU pragma: export
#include "google/protobuf/extension_set.h"  // IWYU pragma: export
#include "google/protobuf/generated_enum_reflection.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"

#define PROTOBUF_INTERNAL_EXPORT_msgid_2eproto

namespace google {
namespace protobuf {
namespace internal {
class AnyMetadata;
}  // namespace internal
}  // namespace protobuf
}  // namespace google

// Internal implementation detail -- do not use these members.
struct TableStruct_msgid_2eproto {
  static const ::uint32_t offsets[];
};
extern const ::google::protobuf::internal::DescriptorTable
    descriptor_table_msgid_2eproto;
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

namespace netmsg {
enum NetCmdID : int {
  CmdNone = 0,
  Ping = 1,
  Pong = 2,
  SecurityInit = 3,
  ReqReg = 81,
  RspReg = 82,
  ReqLogin = 83,
  RspLogin = 84,
  NetCmdID_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  NetCmdID_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool NetCmdID_IsValid(int value);
extern const uint32_t NetCmdID_internal_data_[];
constexpr NetCmdID NetCmdID_MIN = static_cast<NetCmdID>(0);
constexpr NetCmdID NetCmdID_MAX = static_cast<NetCmdID>(84);
constexpr int NetCmdID_ARRAYSIZE = 84 + 1;
const ::google::protobuf::EnumDescriptor*
NetCmdID_descriptor();
template <typename T>
const std::string& NetCmdID_Name(T value) {
  static_assert(std::is_same<T, NetCmdID>::value ||
                    std::is_integral<T>::value,
                "Incorrect type passed to NetCmdID_Name().");
  return ::google::protobuf::internal::NameOfEnum(NetCmdID_descriptor(), value);
}
inline bool NetCmdID_Parse(absl::string_view name, NetCmdID* value) {
  return ::google::protobuf::internal::ParseNamedEnum<NetCmdID>(
      NetCmdID_descriptor(), name, value);
}

// ===================================================================



// ===================================================================




// ===================================================================


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)
}  // namespace netmsg


namespace google {
namespace protobuf {

template <>
struct is_proto_enum<::netmsg::NetCmdID> : std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor<::netmsg::NetCmdID>() {
  return ::netmsg::NetCmdID_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_INCLUDED_msgid_2eproto_2epb_2eh
