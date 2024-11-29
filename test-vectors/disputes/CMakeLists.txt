#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_test_vector(disputes tiny full)

add_test_vector_libraries(disputes
    jam_crust::jam_crust
    PkgConfig::libb2
    schnorrkel_crust::schnorrkel_crust
)

