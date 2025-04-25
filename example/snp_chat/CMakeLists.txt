#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_executable(example_snp_chat
  main.cpp
)
target_link_libraries(example_snp_chat
  logger
  snp
)

