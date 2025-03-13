#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

# Create a shared module library
add_library(example_module MODULE # or SHARED
    example.cpp
)

# Set C++ standard
target_compile_features(example_module PRIVATE
    cxx_std_20
)

# Set include directories
target_include_directories(example_module PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set exported symbols visibility
set_target_properties(example_module PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Ensure exported symbols are explicitly marked
target_compile_definitions(example_module PRIVATE
    EXAMPLE_EXPORTS
)

# Link with libs
target_link_libraries(example_module
    qtils::qtils
)
