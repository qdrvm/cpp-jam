/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/shared_ref.hpp>

#include "blockchain/genesis_block_header.hpp"

namespace jam::app {
  class ChainSpec;
}
namespace jam::log {
  class LoggingSystem;
}
namespace jam::crypto {
  class Hasher;
}

namespace jam::blockchain {

  class GenesisBlockHeaderImpl final : public GenesisBlockHeader {
   public:
    GenesisBlockHeaderImpl(const qtils::SharedRef<log::LoggingSystem> &logsys,
                           const qtils::SharedRef<app::ChainSpec> &chain_spec,
                           const qtils::SharedRef<crypto::Hasher> &hasher);
  };

}  // namespace jam::blockchain
