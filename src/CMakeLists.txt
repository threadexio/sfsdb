# Networking
add_subdirectory(include/nio/)
add_subdirectory(include/resp/)

include_directories(include/)

add_executable(server server/server.cpp)
target_link_libraries(server nio resp)

add_executable(client client/client.cpp)
target_link_libraries(client nio resp)