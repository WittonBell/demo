cmake_minimum_required(VERSION 3.28)
project(cxx20)

set(CMAKE_CXX_STANDARD 23)

if (CMAKE_CXX_COMPILER MATCHES "clang")
message(STATUS "Compiler is clang++")
add_compile_options(
	-gdwarf-4
	-stdlib=libc++
	#-D_LIBCPP_DEBUG=1
)
endif()

add_subdirectory(stdm)
add_subdirectory(libA)
add_subdirectory(libB)
add_subdirectory(libC)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE stdm libC libB libA)
if (CMAKE_CXX_COMPILER MATCHES "clang")
target_link_libraries(${PROJECT_NAME} PRIVATE c++)
endif()
