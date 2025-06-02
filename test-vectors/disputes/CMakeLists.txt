#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

find_package(schnorrkel_crust CONFIG REQUIRED)

add_test_vector(disputes tiny full)

add_test_vector_libraries(disputes
    PkgConfig::libb2
    schnorrkel_crust::schnorrkel_crust
)

