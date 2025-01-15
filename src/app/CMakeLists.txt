#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_library(app_configurator SHARED configurator.cpp)
target_link_libraries(app_configurator
  yaml-cpp::yaml-cpp)

add_library(app_configuration SHARED configuration.cpp)
#target_link_libraries(app_configuration
#  yaml-cpp::yaml-cpp)

add_library(app_state_manager SHARED impl/state_manager_impl.cpp)
target_link_libraries(app_state_manager
    logger
)

add_library(application SHARED impl/application_impl.cpp)
target_link_libraries(application
  qtils::qtils
  soralog::soralog
  app_configuration
)
