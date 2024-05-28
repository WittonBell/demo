cmake_minimum_required(VERSION 3.12.0)
project(pkg_demo VERSION 0.1.0)

enable_testing()
add_executable(${PROJECT_NAME} main.c)
find_package(PkgConfig REQUIRED)
pkg_check_modules (GLIB2 REQUIRED IMPORTED_TARGET glib-2.0>=2.70)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::GLIB2)
