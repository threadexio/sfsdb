cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

project(nio CXX)

file(GLOB_RECURSE NIO_SRC "src/*.cpp")

add_library(nio STATIC ${NIO_SRC})
target_include_directories(nio PRIVATE .)

option(NIO_BUILD_TESTS "Build tests for nio" OFF)

if(NIO_BUILD_TESTS)
	add_subdirectory(tests/)
endif()