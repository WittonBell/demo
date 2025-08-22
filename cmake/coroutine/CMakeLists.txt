cmake_minimum_required(VERSION 3.25)

project(t)

if(MSVC)
add_compile_options("/utf-8")
endif()

add_executable(${PROJECT_NAME} "main.c" "co/co.c")
