// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: enums.proto
// Protobuf C++ Version: 5.28.3

#ifndef GOOGLE_PROTOBUF_INCLUDED_enums_2eproto_2epb_2eh
#define GOOGLE_PROTOBUF_INCLUDED_enums_2eproto_2epb_2eh

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

#define PROTOBUF_INTERNAL_EXPORT_enums_2eproto

namespace google {
namespace protobuf {
namespace internal {
class AnyMetadata;
}  // namespace internal
}  // namespace protobuf
}  // namespace google

// Internal implementation detail -- do not use these members.
struct TableStruct_enums_2eproto {
  static const ::uint32_t offsets[];
};
extern const ::google::protobuf::internal::DescriptorTable
    descriptor_table_enums_2eproto;
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

namespace netmsg {
enum EUserErrorType : int {
  UserETNone = 0,
  UserETOK = 1,
  UserETNameExist = 2,
  UserETNameOrPwdError = 3,
  UserETNameNotExist = 4,
  UserETRegFailed = 5,
  UserETTypeNotExist = 6,
  UserETReconnectFail = 7,
  UserETReconnectOK = 8,
  UserAnotherClientLogin = 9,
  EUserErrorType_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  EUserErrorType_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool EUserErrorType_IsValid(int value);
extern const uint32_t EUserErrorType_internal_data_[];
constexpr EUserErrorType EUserErrorType_MIN = static_cast<EUserErrorType>(0);
constexpr EUserErrorType EUserErrorType_MAX = static_cast<EUserErrorType>(9);
constexpr int EUserErrorType_ARRAYSIZE = 9 + 1;
const ::google::protobuf::EnumDescriptor*
EUserErrorType_descriptor();
template <typename T>
const std::string& EUserErrorType_Name(T value) {
  static_assert(std::is_same<T, EUserErrorType>::value ||
                    std::is_integral<T>::value,
                "Incorrect type passed to EUserErrorType_Name().");
  return EUserErrorType_Name(static_cast<EUserErrorType>(value));
}
template <>
inline const std::string& EUserErrorType_Name(EUserErrorType value) {
  return ::google::protobuf::internal::NameOfDenseEnum<EUserErrorType_descriptor,
                                                 0, 9>(
      static_cast<int>(value));
}
inline bool EUserErrorType_Parse(absl::string_view name, EUserErrorType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<EUserErrorType>(
      EUserErrorType_descriptor(), name, value);
}
enum EUserType : int {
  UserTypeGeneral = 0,
  UserTypeGuest = 1,
  UserTypeThird = 2,
  EUserType_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  EUserType_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool EUserType_IsValid(int value);
extern const uint32_t EUserType_internal_data_[];
constexpr EUserType EUserType_MIN = static_cast<EUserType>(0);
constexpr EUserType EUserType_MAX = static_cast<EUserType>(2);
constexpr int EUserType_ARRAYSIZE = 2 + 1;
const ::google::protobuf::EnumDescriptor*
EUserType_descriptor();
template <typename T>
const std::string& EUserType_Name(T value) {
  static_assert(std::is_same<T, EUserType>::value ||
                    std::is_integral<T>::value,
                "Incorrect type passed to EUserType_Name().");
  return EUserType_Name(static_cast<EUserType>(value));
}
template <>
inline const std::string& EUserType_Name(EUserType value) {
  return ::google::protobuf::internal::NameOfDenseEnum<EUserType_descriptor,
                                                 0, 2>(
      static_cast<int>(value));
}
inline bool EUserType_Parse(absl::string_view name, EUserType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<EUserType>(
      EUserType_descriptor(), name, value);
}
enum EChannelType : int {
  CTDefault = 0,
  Third = 1,
  EChannelType_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  EChannelType_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool EChannelType_IsValid(int value);
extern const uint32_t EChannelType_internal_data_[];
constexpr EChannelType EChannelType_MIN = static_cast<EChannelType>(0);
constexpr EChannelType EChannelType_MAX = static_cast<EChannelType>(1);
constexpr int EChannelType_ARRAYSIZE = 1 + 1;
const ::google::protobuf::EnumDescriptor*
EChannelType_descriptor();
template <typename T>
const std::string& EChannelType_Name(T value) {
  static_assert(std::is_same<T, EChannelType>::value ||
                    std::is_integral<T>::value,
                "Incorrect type passed to EChannelType_Name().");
  return EChannelType_Name(static_cast<EChannelType>(value));
}
template <>
inline const std::string& EChannelType_Name(EChannelType value) {
  return ::google::protobuf::internal::NameOfDenseEnum<EChannelType_descriptor,
                                                 0, 1>(
      static_cast<int>(value));
}
inline bool EChannelType_Parse(absl::string_view name, EChannelType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<EChannelType>(
      EChannelType_descriptor(), name, value);
}
enum ETipType : int {
  TipTypeUp = 0,
  ETipType_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  ETipType_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool ETipType_IsValid(int value);
extern const uint32_t ETipType_internal_data_[];
constexpr ETipType ETipType_MIN = static_cast<ETipType>(0);
constexpr ETipType ETipType_MAX = static_cast<ETipType>(0);
constexpr int ETipType_ARRAYSIZE = 0 + 1;
const ::google::protobuf::EnumDescriptor*
ETipType_descriptor();
template <typename T>
const std::string& ETipType_Name(T value) {
  static_assert(std::is_same<T, ETipType>::value ||
                    std::is_integral<T>::value,
                "Incorrect type passed to ETipType_Name().");
  return ETipType_Name(static_cast<ETipType>(value));
}
template <>
inline const std::string& ETipType_Name(ETipType value) {
  return ::google::protobuf::internal::NameOfDenseEnum<ETipType_descriptor,
                                                 0, 0>(
      static_cast<int>(value));
}
inline bool ETipType_Parse(absl::string_view name, ETipType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ETipType>(
      ETipType_descriptor(), name, value);
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
struct is_proto_enum<::netmsg::EUserErrorType> : std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor<::netmsg::EUserErrorType>() {
  return ::netmsg::EUserErrorType_descriptor();
}
template <>
struct is_proto_enum<::netmsg::EUserType> : std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor<::netmsg::EUserType>() {
  return ::netmsg::EUserType_descriptor();
}
template <>
struct is_proto_enum<::netmsg::EChannelType> : std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor<::netmsg::EChannelType>() {
  return ::netmsg::EChannelType_descriptor();
}
template <>
struct is_proto_enum<::netmsg::ETipType> : std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor<::netmsg::ETipType>() {
  return ::netmsg::ETipType_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_INCLUDED_enums_2eproto_2epb_2eh
