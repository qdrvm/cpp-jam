#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Executables (should contain `main()` function)
add_subdirectory(executable)

# Application's thinks
add_subdirectory(app)

# Dependency injection
add_subdirectory(injector)

# Logging subsystem
add_subdirectory(log)

# Metrics subsystem
add_subdirectory(metrics)

# Clocks and time subsystem
add_subdirectory(clock)

# Merkle tree DB
add_subdirectory(merkle_tree)
