#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Application's things
add_subdirectory(app)

# Clocks and time subsystem
add_subdirectory(clock)

# Executables (should contain `main()` function)
add_subdirectory(executable)

# Dependency injection
add_subdirectory(injector)

# Logging subsystem
add_subdirectory(log)

# Metrics subsystem
add_subdirectory(metrics)

# Modules subsystem
add_subdirectory(modules)

# Subscription Engine subsystem
add_subdirectory(se)

# Simple Network Protocol
add_subdirectory(snp)
