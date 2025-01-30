cmake_minimum_required(VERSION 3.10)
project(HelloWorldTest)

set(CMAKE_CXX_STANDARD 11)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(hello_world_test hello_world_test.cpp)

find_package(GTest REQUIRED)
target_link_libraries(hello_world_test GTest::GTest GTest::Main)