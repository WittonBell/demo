cmake_minimum_required(VERSION 3.25.0)
project(net VERSION 0.1.0 LANGUAGES C CXX)

if(WIN32)
cmake_policy(SET CMP0167 NEW)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules (GLIB2 REQUIRED IMPORTED_TARGET glib-2.0>=2.70)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "protobuf Version：${protobuf_VERSION}")

find_package(absl REQUIRED)

find_package(Boost REQUIRED)
add_library(${PROJECT_NAME} net.cc)
message(STATUS "Boost Version：${Boost_VERSION}")
message(STATUS "Boost Include：${Boost_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib)
if(WIN32)
target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 mswsock
protobuf::libprotobuf
#absl::log
#absl::log_internal_check_op
#boost_log-mt
)
else()
target_link_libraries(${PROJECT_NAME} PRIVATE boost_system)
endif()
