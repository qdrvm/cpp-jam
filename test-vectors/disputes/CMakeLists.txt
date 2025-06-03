#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_test_vector(disputes tiny full)

add_test_vector_libraries(disputes
    ark_vrf::ark_vrf
    PkgConfig::libb2
    schnorrkel::schnorrkel
)

