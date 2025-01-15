cmake_minimum_required(VERSION 3.25.0)
project(cobj VERSION 0.1.0)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
add_compile_options(
	-fms-extensions
	-Wno-microsoft-anon-tag
)
endif()

add_executable(${PROJECT_NAME} ${SRC} shape.c shape.cc main.c)
