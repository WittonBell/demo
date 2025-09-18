# 要想支持`import std`，必须要求CMake最低为3.30版本
cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

# 必须设置当前使用的CMake版本匹配的GUID标识，才能启用`import std`的实验性设置。
# 最新的GUID标识可以查看 https://github.com/Kitware/CMake/blob/master/Help/dev/experimental.rst#c-import-std-support,
# 各个版本的GUID标识需要将`master`替换成当前使用的CMake版本号，比如目前VS2022使用的CMake 3.31.6的需要看：
# https://github.com/Kitware/CMake/blob/v3.31.6/Help/dev/experimental.rst#c-import-std-support
if(CMAKE_VERSION EQUAL 4.1.1)
set(CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444") # CMake 4.1.1的GUID
elseif(CMAKE_VERSION EQUAL 3.31.6)
set(CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508") # CMake 3.31.6的GUID
else()
message(FATAL_ERROR "CXX_IMPORT_STD Required")
endif()

# 这是关键设置，要求在project语句之前设置。
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD ${CXX_IMPORT_STD})

# 这段命令必须在project语句之前
# 检测到是Clang编译器，则使用libc++库。
# 这里必须检测CMAKE_CXX_COMPILER变量，不能使用CMAKE_CXX_COMPILER_ID，
# CMAKE_CXX_COMPILER_ID需要在project命令之后才能使用。
if (CMAKE_CXX_COMPILER MATCHES ".*clang\\+\\+(\\.exe)?$")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

project(import_std)

# 必须启用C++标准模块
set(CMAKE_CXX_MODULE_STD ON)
# 必须使用C++23标准
set(CMAKE_CXX_STANDARD 23)

add_executable(import_std main.cc)
