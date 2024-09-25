#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

function(asn1 name)
  set(ASN_DIR ${PROJECT_SOURCE_DIR}/test-vectors/jamtestvectors/${name})
  foreach(x IN LISTS ARGN)
    list(APPEND ASN_FILES ${ASN_DIR}/${x}.asn)
  endforeach()
  set(OUTPUT_PREFIX ${PROJECT_SOURCE_DIR}/test-vectors/${name}/types)
  set(OUTPUT ${OUTPUT_PREFIX}.hpp ${OUTPUT_PREFIX}.scale.hpp ${OUTPUT_PREFIX}.diff.hpp)
  set(ASN1_PY ${PROJECT_SOURCE_DIR}/python/asn1.py)
  add_custom_command(
    OUTPUT ${OUTPUT}
    COMMAND ${Python3_EXECUTABLE} ${ASN1_PY} ${name}
    DEPENDS ${ASN1_PY} ${ASN_FILES}
  )

  add_library(test_vectors_${name}_types INTERFACE ${OUTPUT})
  target_link_libraries(test_vectors_${name}_types INTERFACE
    scale::scale
  )

  add_executable(test_vectors_${name}_types_test
    types.test.cpp
  )
  target_compile_definitions(test_vectors_${name}_types_test PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
  target_link_libraries(test_vectors_${name}_types_test
    fmt::fmt
    ${GTEST_DEPS}
    test_vectors_${name}_types
  )
  add_test(test_vectors_${name}_types_test test_vectors_${name}_types_test)

  add_executable(test_vectors_${name}_test
    ${name}.test.cpp
  )
  target_compile_definitions(test_vectors_${name}_test PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
  target_link_libraries(test_vectors_${name}_test
    fmt::fmt
    ${GTEST_DEPS}
    headers
    test_vectors_${name}_types
  )
  add_test(test_vectors_${name}_test test_vectors_${name}_test)

endfunction()
