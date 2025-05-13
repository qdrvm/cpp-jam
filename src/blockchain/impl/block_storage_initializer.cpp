/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */


#include "blockchain/impl/block_storage_initializer.hpp"

#include <log/logger.hpp>
#include <qtils/error_throw.hpp>
#include <storage/spaced_storage.hpp>

#include "blockchain/block_storage_error.hpp"
#include "blockchain/impl/block_storage_impl.hpp"
#include "blockchain/impl/storage_util.hpp"
#include "jam_types/block.hpp"

namespace jam::blockchain {

  BlockStorageInitializer::BlockStorageInitializer(
      qtils::SharedRef<log::LoggingSystem> logsys,
      qtils::SharedRef<storage::SpacedStorage> storage,
      storage::trie::RootHash state_root,
      qtils::SharedRef<crypto::Hasher> hasher) {
    // temporary instance of block storage
    BlockStorageImpl block_storage(std::move(logsys), storage, hasher, {});

    // Try to get hash of the genesis block (block #0)
    auto hash_opt_res = blockHashByNumber(*storage, 0);
    if (hash_opt_res.has_error()) {
      block_storage.logger_->critical(
          "Database error at check existing genesis block: {}",
          hash_opt_res.error());
      qtils::raise(hash_opt_res.error());
    }
    auto hash_opt = hash_opt_res.value();

    if (not hash_opt.has_value()) {
      // genesis block initialization
      Block genesis_block;
      genesis_block.header.parent = {};             // no parent
      genesis_block.header.parent_state_root = {};  // no parent
      genesis_block.header.extrinsic_hash = {};     // no extrinsic
      genesis_block.header.slot = 0;                // genesis
      // the rest of the fields have a default value

      // Calculate and save hash
      calculateBlockHash(genesis_block.header, *hasher);

      auto genesis_block_hash_res = block_storage.putBlock(genesis_block);
      if (genesis_block_hash_res.has_error()) {
        block_storage.logger_->critical(
            "Database error at store genesis block into: {}",
            genesis_block_hash_res.error());
        qtils::raise(genesis_block_hash_res.error());
      }
      const auto &genesis_block_hash = genesis_block_hash_res.value();

      auto assignment_res =
          block_storage.assignHashToSlot(BlockInfo{0, genesis_block_hash});
      if (assignment_res.has_error()) {
        block_storage.logger_->critical(
            "Database error at assigning genesis block hash: {}",
            assignment_res.error());
        qtils::raise(assignment_res.error());
      }

      auto sel_leaves_res =
          block_storage.setBlockTreeLeaves({genesis_block_hash});
      if (sel_leaves_res.has_error()) {
        block_storage.logger_->critical(
            "Database error at set genesis block as leaf: {}",
            sel_leaves_res.error());
        qtils::raise(sel_leaves_res.error());
      }

      block_storage.logger_->info(
          "Genesis block {}, state {}", genesis_block_hash, state_root);
    } else {
      auto res = block_storage.hasBlockHeader(hash_opt.value());
      qtils::raise_on_err(res);
      if (not res.value()) {
        block_storage.logger_->critical(
            "Database is not consistent: Genesis block header not found, "
            "but exists num-to-hash record for block #0");
        qtils::raise(BlockStorageError::HEADER_NOT_FOUND);
      }
    }
  }

}  // namespace jam::blockchain
