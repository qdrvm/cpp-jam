#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

jam_add_test(snp_test snp_test.cpp)
target_link_libraries(snp_test
    logger
    snp
)
