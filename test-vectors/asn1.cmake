#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

# Generating constants
if (NOT TARGET generate_constants)
    set(ASN_DIR ${PROJECT_SOURCE_DIR}/test-vectors/jamtestvectors)
    set(ASN1_PY ${PROJECT_SOURCE_DIR}/python/asn1.py)

    file(GLOB ASN_FILES "${ASN_DIR}/jam-types-asn/*-const.asn")

    add_custom_target(generate_constants
        COMMENT "Building generated files..."
    )
    add_custom_command(TARGET generate_constants
        PRE_BUILD
        COMMAND ${Python3_EXECUTABLE} ${ASN1_PY} constants
        DEPENDS ${ASN1_PY} ${ASN_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating constants files: ${CMAKE_CURRENT_BINARY_DIR}"
    )
endif ()

# Generating JAM-types
if (NOT TARGET generate_common_types)
    set(ASN_DIR ${PROJECT_SOURCE_DIR}/test-vectors/jamtestvectors)
    set(ASN1_PY ${PROJECT_SOURCE_DIR}/python/asn1.py)

    file(GLOB ASN_FILES "${ASN_DIR}/jam-types-asn/jam-types.asn")

    add_custom_target(generate_common_types
        COMMENT "Building generated files..."
    )
    add_dependencies(generate_common_types generate_constants)
    add_custom_command(TARGET generate_common_types
        PRE_BUILD
        COMMAND ${Python3_EXECUTABLE} ${ASN1_PY} types
        DEPENDS ${ASN1_PY} ${ASN_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating types files: ${CMAKE_CURRENT_BINARY_DIR}"
    )
endif ()

# Generating module-dependent types
function(generate_from_asn1 name)
    set(TARGET_NAME generate_${name}_types)

    if (TARGET ${TARGET_NAME})
        return()
    endif()

    set(ASN_DIR ${PROJECT_SOURCE_DIR}/test-vectors/jamtestvectors)
    set(ASN1_PY ${PROJECT_SOURCE_DIR}/python/asn1.py)

    file(GLOB ASN_FILES "${ASN_DIR}/${name}/${name}.asn")

    add_custom_target(${TARGET_NAME}
        COMMENT "Building generated files..."
    )
    add_dependencies(${TARGET_NAME} generate_common_types)
    add_custom_command(TARGET ${TARGET_NAME}
        PRE_BUILD
        COMMAND ${Python3_EXECUTABLE} ${ASN1_PY} ${name}
        DEPENDS ${ASN1_PY} ${ASN_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating types files: ${CMAKE_CURRENT_BINARY_DIR}"
    )
endfunction()


function(add_test_vector name)
    set(TEST_VECTOR_${name} 1 PARENT_SCOPE)
    generate_from_asn1(${name})

    set(VECTOR_DIR ${PROJECT_SOURCE_DIR}/test-vectors/${name})

    file(GLOB HPP_FILES_ "${VECTOR_DIR}/*.hpp")
    set(HPP_FILES)
    set(REGEX "${name}-(types|scale|diff).*\\.hpp$")
    foreach(FILE ${HPP_FILES_})
        string(REGEX MATCH ${REGEX} MATCH_RESULT ${FILE})
        if(MATCH_RESULT)
            list(APPEND HPP_FILES ${FILE})
        endif()
    endforeach()

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
        headers
        ${TEST_VECTOR}__types
    )
    add_test(${TEST_VECTOR}__transition_test ${TEST_VECTOR}__transition_test)
endfunction()


function(add_test_vector_libraries name)
    if (NOT TEST_VECTOR_${name})
        message(FATAL_ERROR "Call 'add_test_vector(${name})' first")
    endif()

    set(TEST_VECTOR test_vector__${name})

    target_link_libraries(${TEST_VECTOR}__transition_test ${ARGN})
endfunction()
