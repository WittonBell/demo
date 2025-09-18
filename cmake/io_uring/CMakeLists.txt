cmake_minimum_required(VERSION 3.25)

project(iou)

if(NOT LINUX)
message(FATAL_ERROR "Only linux support iouring")
endif()

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} uring)