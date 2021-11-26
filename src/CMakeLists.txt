# Networking
add_subdirectory(include/nio/)

include_directories(include/)

add_executable(server server.cpp)
target_link_libraries(server nio)

add_executable(client client.cpp)
target_link_libraries(client nio)