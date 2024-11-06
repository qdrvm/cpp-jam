add_library(merkle_tree merkle_tree.cpp)
target_include_directories(merkle_tree 
    PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)
target_link_libraries(merkle_tree PUBLIC blake2b Tracy::TracyClient)
add_library(morum::merkle_tree ALIAS merkle_tree)
if (MORUM_TRACE)
    add_compile_definitions(merkle_tree PUBLIC MORUM_ENABLE_TRACE)
endif()
if (QTILS_ASSERT)
    add_compile_definitions(merkle_tree PUBLIC MORUM_ENABLE_ASSERT)
endif()
target_link_libraries(merkle_tree 
    PUBLIC qtils::qtils 
    PRIVATE RocksDB::rocksdb
)
