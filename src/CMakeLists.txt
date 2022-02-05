# Networking
add_subdirectory(include/nio/)
include_directories(include/nio/include/)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.71.0 COMPONENTS program_options REQUIRED)

include_directories(./)

file(GLOB SRC "*.cpp")
file(GLOB HANDLER_SRC "handlers/*.cpp")

add_library(main OBJECT ${SRC})

add_executable(sfsdb-server ${HANDLER_SRC} server/server.cpp)
target_link_libraries(sfsdb-server main NioStatic pthread Boost::program_options)

add_executable(sfsdb-cli client/client.cpp)
target_link_libraries(sfsdb-cli main NioStatic Boost::program_options)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
	add_subdirectory(tests/)
endif()