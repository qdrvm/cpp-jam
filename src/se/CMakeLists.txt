#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_library(se_async
    async_dispatcher.cpp
    subscription.cpp
    )

target_link_libraries(se_async
    )

add_library(se_sync
    sync_dispatcher.cpp
    subscription.cpp
    )

target_link_libraries(se_sync
    )
