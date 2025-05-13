#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

include(${PROJECT_SOURCE_DIR}/test-vectors/asn1.cmake)
add_test_vector(safrole)
add_test_vector_libraries(safrole
    # jam_crust::jam_crust # Удалено из-за отсутствия зависимости
    PkgConfig::libb2
)
