/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/types.tmp.hpp>
#include <qtils/shared_ref.hpp>
#include <utils/ctor_limiters.hpp>

namespace jam::log {
  class LoggingSystem;
}
namespace jam::storage {
  class SpacedStorage;
}
namespace jam::crypto {
  class Hasher;
}
namespace jam::storage::trie {
  using RootHash = test_vectors::OpaqueHash;
}

namespace jam::blockchain {

  class BlockStorageInitializer final : Singleton<BlockStorageInitializer> {
   public:
    BlockStorageInitializer(qtils::SharedRef<log::LoggingSystem> logsys,
                            qtils::SharedRef<storage::SpacedStorage> storage,
                            storage::trie::RootHash state_root,
                            qtils::SharedRef<crypto::Hasher> hasher);
  };

}  // namespace jam::blockchain
