#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#


function(jam_add_test name)
    add_executable(${name}
        ${ARGN}
    )
    target_link_libraries(${name}
        ${GTEST_DEPS}
    )
    add_test(${name} ${name})
endfunction()

add_subdirectory(snp)
