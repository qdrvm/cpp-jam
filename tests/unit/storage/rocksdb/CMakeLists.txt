#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

addtest(rocksdb_fs_test
    rocksdb_fs_test.cpp
)
target_link_libraries(rocksdb_fs_test
    base_fs_test
    storage
)

addtest(rocksdb_integration_test
    rocksdb_integration_test.cpp
)
target_link_libraries(rocksdb_integration_test
    base_rocksdb_test
)
