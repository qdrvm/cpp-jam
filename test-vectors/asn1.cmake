#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

set(ASN_DIR ${PROJECT_SOURCE_DIR}/test-vectors/jamtestvectors)
set(ASN1_PY ${PROJECT_SOURCE_DIR}/scripts/asn1.py)

# Generating constants
if (NOT TARGET generate_constants)
  file(GLOB ASN_FILES "${ASN_DIR}/jam-types-asn/*-const.asn")

  set(TARGET_DIR ${CMAKE_BINARY_DIR}/generated/jam_types)
  file(MAKE_DIRECTORY ${TARGET_DIR})
  set(GENERATED_FILES
      ${TARGET_DIR}/config.hpp
      ${TARGET_DIR}/config-tiny.hpp
      ${TARGET_DIR}/config-full.hpp
      ${TARGET_DIR}/constants-tiny.hpp
      ${TARGET_DIR}/constants-full.hpp
  )

  add_custom_command(
      OUTPUT ${GENERATED_FILES}
      COMMAND ${Python3_EXECUTABLE} ${ASN1_PY} ${TARGET_DIR} constants
      DEPENDS ${ASN1_PY} ${ASN_FILES}
      COMMENT "Generating constants files: ${CMAKE_CURRENT_SOURCE_DIR}"
  )

  add_custom_target(generate_constants
      DEPENDS ${GENERATED_FILES}
      COMMENT "Building generated files..."
  )
endif ()

# Generating JAM-types
if (NOT TARGET generate_common_types)
  file(GLOB ASN_FILES "${ASN_DIR}/jam-types-asn/jam-types.asn")

  set(TARGET_DIR ${CMAKE_BINARY_DIR}/generated/jam_types)
  file(MAKE_DIRECTORY ${TARGET_DIR})
  set(GENERATED_FILES
      ${TARGET_DIR}/common-types.hpp
      ${TARGET_DIR}/common-diff.hpp
  )

  add_custom_command(
      OUTPUT ${GENERATED_FILES}
      COMMAND ${Python3_EXECUTABLE} ${ASN1_PY} ${TARGET_DIR} types
      DEPENDS ${ASN1_PY} ${ASN_FILES}
      COMMENT "Generating types files: ${CMAKE_CURRENT_SOURCE_DIR}"
  )

  add_custom_target(generate_common_types
      DEPENDS generate_constants ${GENERATED_FILES}
      COMMENT "Building generated files..."
  )
endif ()

# Generating module-dependent types
function(generate_from_asn1 name)
  set(TARGET_NAME generate_${name}_types)

  if (TARGET ${TARGET_NAME})
    return()
  endif ()

  file(GLOB ASN_FILES "${ASN_DIR}/${name}/${name}.asn")

  set(TARGET_DIR ${CMAKE_BINARY_DIR}/generated/jam_types)
  file(MAKE_DIRECTORY ${TARGET_DIR})
  set(GENERATED_FILES
      ${TARGET_DIR}/${name}-types.hpp
      ${TARGET_DIR}/${name}-diff.hpp
  )

  message(STATUS "${GENERATED_FILES}")
  add_custom_command(
      OUTPUT ${GENERATED_FILES}
      COMMAND ${Python3_EXECUTABLE} ${ASN1_PY} ${TARGET_DIR} ${name}
      DEPENDS ${ASN1_PY} ${ASN_FILES}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating types files of ${name}: ${CMAKE_CURRENT_SOURCE_DIR}"
  )

  add_custom_target(${TARGET_NAME}
      DEPENDS generate_common_types ${GENERATED_FILES}
      COMMENT "Building generated files..."
  )
endfunction()


function(add_test_vector name)
  set(TEST_VECTOR_${name} 1 PARENT_SCOPE)
  generate_from_asn1(${name})

  set(VECTOR_DIR ${PROJECT_SOURCE_DIR}/test-vectors/${name})

  file(GLOB HPP_FILES_ "${VECTOR_DIR}/*.hpp")
  set(HPP_FILES)
  set(REGEX "${name}-(types|diff).*\\.hpp$")
  foreach (FILE ${HPP_FILES_})
    string(REGEX MATCH ${REGEX} MATCH_RESULT ${FILE})
    if (MATCH_RESULT)
      list(APPEND HPP_FILES ${FILE})
    endif ()
  endforeach ()

  set(TEST_VECTOR test_vector__${name})

  add_library(${TEST_VECTOR}__types INTERFACE ${HPP_FILES})
  target_link_libraries(${TEST_VECTOR}__types INTERFACE
      scale::scale
      test_vectors_headers
  )
  add_dependencies(${TEST_VECTOR}__types generate_${name}_types)

  add_executable(${TEST_VECTOR}__reencode_test
      types.test.cpp
  )
  target_compile_definitions(${TEST_VECTOR}__reencode_test PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
  target_include_directories(${TEST_VECTOR}__reencode_test PUBLIC ${PROJECT_SOURCE_DIR})
  target_link_libraries(${TEST_VECTOR}__reencode_test
      fmt::fmt
      ${GTEST_DEPS}
      ${TEST_VECTOR}__types
  )
  add_test(${TEST_VECTOR}__reencode_test ${TEST_VECTOR}__reencode_test)

  add_executable(${TEST_VECTOR}__transition_test
      ${name}.test.cpp
  )
  target_compile_definitions(${TEST_VECTOR}__transition_test PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
  target_link_libraries(${TEST_VECTOR}__transition_test
      fmt::fmt
      ${GTEST_DEPS}
      ${TEST_VECTOR}__types
  )
  add_test(${TEST_VECTOR}__transition_test ${TEST_VECTOR}__transition_test)
endfunction()


function(add_test_vector_libraries name)
  if (NOT TEST_VECTOR_${name})
    message(FATAL_ERROR "Call 'add_test_vector(${name})' first")
  endif ()

  set(TEST_VECTOR test_vector__${name})

  target_link_libraries(${TEST_VECTOR}__transition_test ${ARGN})
endfunction()
