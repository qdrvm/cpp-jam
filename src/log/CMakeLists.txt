#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_library(logger
    logger.cpp
)
target_link_libraries(logger
    fmt::fmt
    soralog::soralog
    soralog::configurator_yaml
    qtils::qtils
    Boost::algorithm
    )
#kagome_install(logger)

#add_library(log_configurator
#    configurator.cpp
#    )
#target_link_libraries(log_configurator
#    soralog::configurator_yaml
#    Boost::program_options
#    soralog::soralog
#    )
#kagome_install(log_configurator)
