
add_executable(set_get_benchmark merkle_tree/set_get.cpp)
target_link_libraries(set_get_benchmark benchmark::benchmark merkle_tree Tracy::TracyClient)
