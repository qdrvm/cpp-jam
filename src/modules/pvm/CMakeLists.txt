#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_jam_module(pvm
  INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src
  LIBRARIES
    qtils::qtils
    soralog::soralog
) 