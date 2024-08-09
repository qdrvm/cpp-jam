#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

set(SAFROLE_ASN_DIR ${PROJECT_SOURCE_DIR}/test-vectors/jamtestvectors/safrole)
set(SAFROLE_ASN ${SAFROLE_ASN_DIR}/safrole.asn ${SAFROLE_ASN_DIR}/tiny.asn ${SAFROLE_ASN_DIR}/full.asn)
set(SAFROLE_OUTPUT_PREFIX ${PROJECT_SOURCE_DIR}/test-vectors/safrole/types)
set(SAFROLE_OUTPUT ${SAFROLE_OUTPUT_PREFIX}.hpp ${SAFROLE_OUTPUT_PREFIX}.scale.hpp)
set(SAFROLE_PY ${PROJECT_SOURCE_DIR}/python/asn1.py)
add_custom_command(
  OUTPUT ${SAFROLE_OUTPUT}
  COMMAND python3 ${SAFROLE_PY}
  DEPENDS ${SAFROLE_PY} ${SAFROLE_ASN}
)

add_library(test_vectors_safrole_types INTERFACE ${SAFROLE_OUTPUT})
target_link_libraries(test_vectors_safrole_types INTERFACE
  scale::scale
)

add_executable(test_vectors_safrole_types_test
  types.test.cpp
)
target_compile_definitions(test_vectors_safrole_types_test PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(test_vectors_safrole_types_test
  fmt::fmt
  test_vectors_safrole_types
)
add_test(NAME test_vectors_safrole_types_test COMMAND $<TARGET_FILE:test_vectors_safrole_types_test>)
