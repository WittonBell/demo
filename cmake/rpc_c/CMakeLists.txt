cmake_minimum_required(VERSION 3.20)
project(rpc)

set(CMAKE_C_STANDARD_REQUIRED 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules (libprotobuf-c REQUIRED IMPORTED_TARGET libprotobuf-c>=1.0)

aux_source_directory(. SRC)

get_filename_component(proto "login.proto" ABSOLUTE)
get_filename_component(proto_path "${proto}" PATH)

set(proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/login.pb-c.c")
set(proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/login.pb-c.h")

add_custom_command(
      OUTPUT "${proto_srcs}" "${proto_hdrs}"
      COMMAND "protoc"
      ARGS --c_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${proto_path}"
        "${proto}"
      DEPENDS "${proto}")

add_executable(${PROJECT_NAME} ${SRC} ${proto_srcs})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(WIN32)
target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
target_link_libraries(${PROJECT_NAME} PkgConfig::libprotobuf-c)
