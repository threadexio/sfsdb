# Networking
add_subdirectory(include/nio/)

# Resp parser
add_subdirectory(include/resp/)

include_directories(include/)
include_directories(./)

file(GLOB SRC "*.cpp")

add_library(main OBJECT ${SRC})

add_executable(server server/server.cpp)
target_link_libraries(server main nio resp)

add_executable(client client/client.cpp)
target_link_libraries(client main nio resp)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
	add_subdirectory(tests/)
endif()