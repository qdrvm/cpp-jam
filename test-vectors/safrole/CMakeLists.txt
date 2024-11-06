#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

asn1(safrole safrole tiny full)
target_link_libraries(test_vectors_safrole_test
  jam_crust::jam_crust
  PkgConfig::libb2
)
