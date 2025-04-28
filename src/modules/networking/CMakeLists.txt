#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_jam_module(networking
  SOURCE
    networking.cpp
  DEFINITIONS
    SOME_FLAG=1
  LIBRARIES
    qtils::qtils
    soralog::soralog
    snp
)