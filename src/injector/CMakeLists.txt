#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_library(node_injector
    node_injector.cpp
)
target_link_libraries(node_injector
    app_configurator
    app_state_manager
    application
    Boost::Boost.DI
    clock
    logger
    metrics
    modules
    se_async
    snp
)
