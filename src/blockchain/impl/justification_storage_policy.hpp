/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/block_header.hpp"
#include "qtils/outcome.hpp"

namespace jam::blockchain {
  class BlockTree;
}

namespace jam::blockchain {

  class JustificationStoragePolicy {
   public:
    virtual ~JustificationStoragePolicy() = default;

    [[nodiscard]] virtual outcome::result<bool> shouldStoreFor(
        const BlockHeader &block,
        BlockNumber last_finalized_number) const = 0;
  };

  class JustificationStoragePolicyImpl final
      : public JustificationStoragePolicy {
   public:
    [[nodiscard]] outcome::result<bool> shouldStoreFor(
        const BlockHeader &block,
        BlockNumber last_finalized_number) const override;
  };

}  // namespace kagome::blockchain
