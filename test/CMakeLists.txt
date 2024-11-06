
morum_add_test(codec_test merkle_tree/codec_test.cpp)
target_link_libraries(codec_test qtils::qtils)

morum_add_test(compatibility_test merkle_tree/compat_test.cpp)
target_include_directories(compatibility_test 
    PUBLIC 
    third_party/json/single_include
)
target_link_libraries(compatibility_test merkle_tree nlohmann_json::nlohmann_json qtils::qtils)

morum_add_test(tree_test merkle_tree/tree_test.cpp)
target_link_libraries(tree_test merkle_tree RocksDB::rocksdb Tracy::TracyClient)

morum_add_test(primitives_test merkle_tree/primitives_test.cpp)
target_link_libraries(primitives_test merkle_tree)
