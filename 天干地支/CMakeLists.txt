cmake_minimum_required(VERSION 3.30)
#年份转天干地支纪年
project(tgdz)

aux_source_directory(. SRC)

add_executable(${PROJECT_NAME} ${SRC})
