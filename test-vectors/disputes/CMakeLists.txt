#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

asn1(disputes disputes tiny full)
target_link_libraries(test_vectors_disputes_test
    jam_crust::jam_crust
    schnorrkel_crust::schnorrkel_crust
    PkgConfig::libb2
)
