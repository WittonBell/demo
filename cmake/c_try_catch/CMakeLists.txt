cmake_minimum_required(VERSION 3.30)
project(t)


add_subdirectory(ctry)

aux_source_directory(. src)

add_executable(${PROJECT_NAME} ${src})
target_link_libraries(${PROJECT_NAME} ctry)
