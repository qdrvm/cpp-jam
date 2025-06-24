/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/block_header.hpp>
#include <qtils/shared_ref.hpp>
#include <utils/ctor_limiters.hpp>

namespace jam::log {
  class LoggingSystem;
}
namespace jam::blockchain {
  class BlockStorage;
}

namespace jam::blockchain {

  class BlockTreeInitializer final : Singleton<BlockTreeInitializer> {
   public:
    BlockTreeInitializer(qtils::SharedRef<log::LoggingSystem> logsys,
                         qtils::SharedRef<BlockStorage> storage);

    std::tuple<BlockIndex, std::map<BlockIndex, BlockHeader>>
    nonFinalizedSubTree();

   private:
    std::atomic_flag used_;
    BlockIndex last_finalized_;
    std::map<BlockIndex, BlockHeader> non_finalized_;
  };

}  // namespace jam::blockchain
