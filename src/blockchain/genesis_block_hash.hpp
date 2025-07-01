/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "primitives/common.hpp"

namespace jam::blockchain {

  class BlockTree;

  class GenesisBlockHash final : public primitives::BlockHash {
   public:
    GenesisBlockHash(std::shared_ptr<BlockTree> block_tree);
  };

}  // namespace jam::blockchain

template <>
struct fmt::formatter<jam::blockchain::GenesisBlockHash>
    : fmt::formatter<qtils::ByteView> {};
