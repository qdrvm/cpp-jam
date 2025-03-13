#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_library(modules
    module_loader.cpp
    )

target_link_libraries(modules
    qtils::qtils
    )

# -------------- Modules --------------

# Example module
add_subdirectory(example)