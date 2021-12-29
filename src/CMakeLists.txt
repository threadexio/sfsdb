# Networking
add_subdirectory(include/nio/)
add_subdirectory(include/resp/)

include_directories(include/)
include_directories(./)

file(GLOB SRC "*.cpp")

add_executable(server ${SRC} server/server.cpp)
target_link_libraries(server nio resp)

add_executable(client ${SRC} client/client.cpp)
target_link_libraries(client nio resp)