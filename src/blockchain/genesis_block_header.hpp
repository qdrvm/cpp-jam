/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/block_header.hpp"

namespace jam::blockchain {

  class GenesisBlockHeader : public BlockHeader {
   public:
    using BlockHeader::BlockHeader;
  };

}  // namespace jam::blockchain
