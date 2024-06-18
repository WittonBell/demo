cmake_minimum_required(VERSION 3.25.0)
project(bigint VERSION 0.1.0 LANGUAGES C)

if(MSVC)
	add_compile_options("/utf-8")
endif()
include_directories(SYSTEM "inc")
aux_source_directory("src" SRC)
add_library(bigint ${SRC})

add_executable(t test.c)
target_link_libraries(t bigint)

