#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

# fix for dyld bug on mac
# we should actually check the target platform here, not the host platform,
# but not that we explicitly support cross-compilation anyway
if (CMAKE_HOST_APPLE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_library(jam_node SHARED jam_node.cpp)
  set_target_properties(jam_node PROPERTIES PREFIX "" DEBUG_POSTFIX "")

  add_executable(jam_node_dlopen dlopen.c)
  set_target_properties(jam_node_dlopen PROPERTIES OUTPUT_NAME jam_node)
  set_target_properties(jam_node_dlopen PROPERTIES LINKER_LANGUAGE CXX)
else ()
  add_executable(jam_node jam_node.cpp)
endif ()

target_link_libraries(jam_node
#    Boost::boost
#    app_configurator
    node_injector
#    Boost::program_options
#    kagome_application
#    app_config
#    fd_limit
#    p2p::p2p_identify
#    p2p::p2p_ping
#    kagome-benchmark
#    kagome-db-editor
#    storage_explorer
#    filesystem
#    kagome_key
    )

if (BACKWARD)
  add_backward(jam_node)
endif ()
