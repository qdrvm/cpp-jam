/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "primitives/common.hpp"

namespace lean::blockchain {

  class BlockTree;

  class GenesisBlockHash final : public primitives::BlockHash {
   public:
    GenesisBlockHash(std::shared_ptr<BlockTree> block_tree);
  };

}  // namespace lean::blockchain

template <>
struct fmt::formatter<lean::blockchain::GenesisBlockHash>
    : fmt::formatter<qtils::ByteView> {};
