# Networking
add_subdirectory(nio/)

include_directories(include/)

add_executable(main main.cpp)
target_link_libraries(main nio)