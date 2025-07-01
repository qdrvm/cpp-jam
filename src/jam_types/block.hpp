/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/block_header.hpp>
#include <jam_types/extricsic.hpp>

namespace jam {

  struct Block {
    BlockHeader header;
    BlockBody extrinsic;
    bool operator==(const Block &) const = default;
  };

}  // namespace jam
