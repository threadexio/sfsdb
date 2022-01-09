# Networking
add_subdirectory(include/nio/)
include_directories(include/nio/include/)

# Resp parser
include_directories(include/redis-cpp/include/)

include_directories(include/)
include_directories(./)

file(GLOB SRC "*.cpp")

add_library(main OBJECT ${SRC})

add_executable(server server/server.cpp)
target_link_libraries(server main NioStatic)

add_executable(client client/client.cpp)
target_link_libraries(client main NioStatic)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
	add_subdirectory(tests/)
endif()