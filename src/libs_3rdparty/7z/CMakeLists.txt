cmake_minimum_required(VERSION 2.8)

project(7zlib)

cmake_minimum_required(VERSION 3.4)
project(zlib)

find_package(Qt5 REQUIRED Core)

include_directories(src)

file(GLOB_RECURSE SRCS src/*.c)
file(GLOB_RECURSE HDRS src/*.h)

add_library(7z STATIC ${HDRS} ${SRCS})

