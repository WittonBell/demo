cmake_minimum_required(VERSION 3.25.0)
project(t VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_C_STANDARD 23)

add_executable(server server_ex.c)
add_executable(client client.c)

target_link_libraries(server ws2_32 Mswsock)
target_link_libraries(client ws2_32)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
