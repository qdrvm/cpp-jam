#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Applications (should contain `main()` function)
add_subdirectory(apps)

# Logging subsystem
add_subdirectory(log)

