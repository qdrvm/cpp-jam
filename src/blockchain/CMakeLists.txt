#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_library(blockchain
    impl/storage_util.cpp
    impl/block_storage_impl.cpp
    impl/block_storage_error.cpp
    impl/block_storage_initializer.cpp
    impl/block_tree_error.cpp
    impl/genesis_block_header_impl.cpp
    impl/cached_tree.cpp
    impl/block_tree_impl.cpp
    impl/block_tree_initializer.cpp
    impl/justification_storage_policy.cpp
#    genesis_block_hash.cpp
    )
target_link_libraries(blockchain
    Boost::boost
#    storage
#    primitives
#    logger
#    blob
#    metrics
#    telemetry
#    outcome
#    hasher
#    scale::scale
#    headers
    )
