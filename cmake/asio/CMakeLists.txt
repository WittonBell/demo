cmake_minimum_required(VERSION 3.5.0)
project(asio VERSION 0.1.0 LANGUAGES C CXX)

add_compile_options(
	-std=c++23
)

aux_source_directory(pb/cpp Src)

add_subdirectory(net)
add_executable(${PROJECT_NAME} ${Src} main.cc)
target_link_libraries(${PROJECT_NAME} net)
