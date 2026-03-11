cmake_minimum_required(VERSION 3.20)
project(rpc)

set(CMAKE_C_STANDARD_REQUIRED 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules (libprotobuf-c REQUIRED IMPORTED_TARGET libprotobuf-c>=1.0)

aux_source_directory(pbc SRC)
aux_source_directory(. SRC)

add_executable(${PROJECT_NAME} ${SRC})
if(WIN32)
target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
target_link_libraries(${PROJECT_NAME} PkgConfig::libprotobuf-c)
