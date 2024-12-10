cmake_minimum_required(VERSION 3.25.0)
project(asio VERSION 0.1.0 LANGUAGES C CXX)

add_compile_options(
	-std=c++23
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
add_compile_options(-fcoroutines)
add_compile_definitions(
	#BOOST_ASIO_ENABLE_HANDLER_TRACKING
	__clang_major__=18
)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
add_compile_options(
	-fclang-abi-compat=17
)
endif()

aux_source_directory(pb/cpp Src)

add_subdirectory(net)
add_executable(${PROJECT_NAME} ${Src} main.cc)
target_link_libraries(${PROJECT_NAME} net)
