cmake_minimum_required(VERSION 3.12.0)
project(pb VERSION 0.1.0)

enable_testing()

include(proto.cmake)
protobuf_generate_cpp(GENERATED_CPP_SRC GENERATED_CPP_HEADER "src/pb/t.proto" "src/pb/x.proto" "a.proto")
protobuf_generate_c(GENERATED_C_SRC GENERATED_C_HEADER "src/pb/t.proto" "src/pb/x.proto" "a.proto")

add_executable(${PROJECT_NAME} main.c ${GENERATED_C_SRC})
target_compile_options(${PROJECT_NAME} PUBLIC -gdwarf-4)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Protobuf protobuf-c)
