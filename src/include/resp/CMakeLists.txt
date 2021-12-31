cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

project(resp CXX)

file(GLOB_RECURSE RESP_SRC "src/*.cpp")

add_library(resp STATIC ${RESP_SRC})
target_include_directories(resp PRIVATE .)

option(RESP_BUILD_TESTS "Build tests for resp" OFF)

if(RESP_BUILD_TESTS)
	add_subdirectory(tests/)
endif()