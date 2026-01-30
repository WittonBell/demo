cmake_minimum_required(VERSION 3.30)
project(ctry)

aux_source_directory(. SRC)

add_library(${PROJECT_NAME} ${SRC})
