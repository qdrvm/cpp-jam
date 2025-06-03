#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_test_vector(safrole tiny full)

add_test_vector_libraries(safrole
    PkgConfig::libb2
    ark_vrf::ark_vrf
)
