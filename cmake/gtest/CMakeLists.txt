cmake_minimum_required(VERSION 3.12.0)
project(gtest_demo VERSION 0.1.0)

enable_testing()

find_package(GTest REQUIRED)

# 支持使用ctest命令进行测试
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

add_executable(${PROJECT_NAME} test.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest GTest::gtest_main)

gtest_discover_tests(${PROJECT_NAME})
