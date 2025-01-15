cmake_minimum_required(VERSION 3.25.0)
project(swissmap VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
if(MSVC)
add_compile_options(/source-charset:utf-8)
endif()

add_executable(${PROJECT_NAME} main.cc)
if(CMAKE_HOST_WIN32)
target_link_libraries(${PROJECT_NAME} PRIVATE winmm.lib)
endif()

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
