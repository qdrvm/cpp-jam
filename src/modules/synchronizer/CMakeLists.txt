#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_jam_module(synchronizer
  SOURCE
    synchronizer.cpp
  DEFINITIONS
    SOME_FLAG=1
  LIBRARIES
    qtils::qtils
    soralog::soralog
)