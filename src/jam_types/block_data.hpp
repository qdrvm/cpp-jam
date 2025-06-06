/**
* Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/block.hpp>

namespace jam {

  using test_vectors::Extrinsic;

  struct BlockData {
    BlockHash hash;
    std::optional<BlockHeader> header;
    std::optional<Extrinsic> extrinsic;
  };

}  // namespace jam
