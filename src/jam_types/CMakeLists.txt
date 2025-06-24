#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_library(jam_types SHARED
    peer_id.cpp
    peer_address.cpp
)
target_link_libraries(jam_types
    qtils::qtils
)
