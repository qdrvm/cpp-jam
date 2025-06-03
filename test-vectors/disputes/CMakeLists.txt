#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

find_package(qdrvm-crates CONFIG REQUIRED)

add_test_vector(disputes tiny full)

add_test_vector_libraries(disputes
    PkgConfig::libb2
    schnorrkel::schnorrkel
)

