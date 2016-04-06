cmake_minimum_required(VERSION 3.4)
project(gtest)

include_directories(src)

file(GLOB_RECURSE SRCS src/*.cc)
file(GLOB_RECURSE HDRS src/*.h)

add_definitions(-DGTEST_CREATE_SHARED_LIBRARY=1)

add_library(gtest SHARED ${HDRS}
        src/gtest-all.cc)

target_link_libraries(gtest)

