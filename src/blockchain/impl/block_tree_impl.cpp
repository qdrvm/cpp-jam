/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "blockchain/impl/block_tree_impl.hpp"

// #include <algorithm>
// #include <set>
// #include <soralog/macro.hpp>
// #include <stack>

#include <qtils/cxx23/ranges/contains.hpp>

#include "blockchain/block_tree_error.hpp"
#include "blockchain/impl/block_tree_initializer.hpp"
#include "blockchain/impl/cached_tree.hpp"
#include "blockchain/impl/justification_storage_policy.hpp"
#include "se/subscription.hpp"
#include "se/subscription_fwd.hpp"
// #include "blockchain/impl/storage_util.hpp"
// #include "common/main_thread_pool.hpp"
// #include "consensus/babe/impl/babe_digests_util.hpp"
// #include "consensus/babe/is_primary.hpp"
// #include "crypto/blake2/blake2b.h"
// #include "log/profiling_logger.hpp"
// #include "storage/database_error.hpp"
// #include "storage/trie_pruner/trie_pruner.hpp"
// #include "utils/pool_handler.hpp"
#include "tests/testutil/literals.hpp"

// namespace {
//   constexpr auto blockHeightMetricName = "jam_block_height";
//   constexpr auto knownChainLeavesMetricName = "jam_number_leaves";
// }  // namespace

namespace jam::blockchain {
  // using StorageError = jam::storage::StorageError;

  namespace {
    // Check if block produced by ticket or fallback way
    bool isPrimary(const BlockHeader &header) {
      // Check the source of seal-keys (γs)
      //    - Z(γa) - by ticket
      //    - F(...) - fallback

      // FIXME calculate real result
      return header.extrinsic_hash == "ticket"_arr32;
    }
  }  // namespace


  BlockTreeImpl::SafeBlockTreeData::SafeBlockTreeData(BlockTreeData data)
      : block_tree_data_{std::move(data)} {}

  // outcome::result<void> BlockTreeImpl::recover(
  //     const BlockId &target_block_id,
  //     std::shared_ptr<BlockStorage> storage,
  //     std::shared_ptr<const storage::trie::TrieStorage> trie_storage,
  //     std::shared_ptr<BlockTree> block_tree) {
  //   BOOST_ASSERT(storage != nullptr);
  //   BOOST_ASSERT(trie_storage != nullptr);
  //
  //   log::Logger log = log::createLogger("BlockTree", "block_tree");
  //
  //   OUTCOME_TRY(block_tree_leaves, loadLeaves(storage, log));
  //
  //   BOOST_ASSERT_MSG(not block_tree_leaves.empty(),
  //                    "Must be known or calculated at least one leaf");
  //
  //   auto target_block_hash_opt_res = storage->getBlockHash(target_block_id);
  //   if (target_block_hash_opt_res.has_failure()) {
  //     SL_CRITICAL(log,
  //                 "Can't get header of target block: {}",
  //                 target_block_hash_opt_res.error());
  //     return BlockTreeError::HEADER_NOT_FOUND;
  //   }
  //   if (not target_block_hash_opt_res.value().has_value()) {
  //     SL_CRITICAL(log, "Can't get header of target block: header not found");
  //     return BlockTreeError::HEADER_NOT_FOUND;
  //   }
  //   const auto &target_block_hash =
  //   target_block_hash_opt_res.value().value();
  //
  //   // Check if target block exists
  //   auto target_block_header_res =
  //   storage->getBlockHeader(target_block_hash); if
  //   (target_block_header_res.has_error()) {
  //     SL_CRITICAL(log,
  //                 "Can't get header of target block: {}",
  //                 target_block_header_res.error());
  //     return target_block_header_res.as_failure();
  //   }
  //
  //   const auto &target_block_header = target_block_header_res.value();
  //   const auto &state_root = target_block_header.state_root;
  //
  //   // Check if target block has state
  //   if (auto res = trie_storage->getEphemeralBatchAt(state_root);
  //       res.has_error()) {
  //     SL_WARN(log, "Can't get state of target block: {}", res.error());
  //     SL_CRITICAL(
  //         log,
  //         "You will need to use `--sync Fast' CLI arg the next time you
  //         start");
  //   }
  //
  //   for (auto it = block_tree_leaves.rbegin(); it !=
  //   block_tree_leaves.rend();
  //        it = block_tree_leaves.rbegin()) {
  //     auto block = *it;
  //     if (target_block_header.number >= block.number) {
  //       break;
  //     }
  //
  //     auto header_res = storage->getBlockHeader(block.hash);
  //     if (header_res.has_error()) {
  //       SL_CRITICAL(log,
  //                   "Can't get header of one of removing block: {}",
  //                   header_res.error());
  //       return header_res.as_failure();
  //     }
  //
  //     const auto &header = header_res.value();
  //     block_tree_leaves.emplace(*header.parentInfo());
  //     block_tree_leaves.erase(block);
  //
  //     std::vector<BlockHash> leaves;
  //     leaves.reserve(block_tree_leaves.size());
  //
  //     std::ranges::transform(block_tree_leaves,
  //                            std::back_inserter(leaves),
  //                            [](const auto it) { return it.hash; });
  //     if (auto res = storage->setBlockTreeLeaves(leaves); res.has_error()) {
  //       SL_CRITICAL(
  //           log, "Can't save updated block tree leaves: {}", res.error());
  //       return res.as_failure();
  //     }
  //
  //     if (auto res = block_tree->removeLeaf(block.hash); res.has_error()) {
  //       SL_CRITICAL(log, "Can't remove block {}: {}", block, res.error());
  //       return res.as_failure();
  //     }
  //   }
  //
  //   return outcome::success();
  // }

  BlockTreeImpl::BlockTreeImpl(
      qtils::SharedRef<log::LoggingSystem> logsys,
      qtils::SharedRef<const app::Configuration> app_config,
      qtils::SharedRef<BlockStorage> storage,
      // const BlockInfo &finalized,
      qtils::SharedRef<crypto::Hasher> hasher,
      // primitives::events::ChainSubscriptionEnginePtr chain_events_engine,
      // primitives::events::ExtrinsicSubscriptionEnginePtr
      //     extrinsic_events_engine,
      // std::shared_ptr<subscription::ExtrinsicEventKeyRepository>
      //     extrinsic_event_key_repo,
      std::shared_ptr<const JustificationStoragePolicy>
          justification_storage_policy,
      // std::shared_ptr<storage::trie_pruner::TriePruner> state_pruner,
      // common::MainThreadPool &main_thread_pool,
      std::shared_ptr<Subscription> se_manager,
      qtils::SharedRef<BlockTreeInitializer> initializer)
      : log_(logsys->getLogger("BlockTree", "block_tree")),
        se_manager_(std::move(se_manager)),
        block_tree_data_{{
            .storage_ = std::move(storage),
            // .state_pruner_ = std::move(state_pruner),
            .tree_ = std::make_unique<CachedTree>(
                std::get<0>(initializer->nonFinalizedSubTree())),
            .hasher_ = std::move(hasher),
            // .extrinsic_event_key_repo_ =
            // std::move(extrinsic_event_key_repo),
            .justification_storage_policy_ =
                std::move(justification_storage_policy),
            // .blocks_pruning_ = {app_config.blocksPruning(),
            //     finalized.number},
        }}  //
  // chain_events_engine_{std::move(chain_events_engine)},
  // main_pool_handler_{main_thread_pool.handlerStarted()},
  // extrinsic_events_engine_{std::move(extrinsic_events_engine)}
  {
    block_tree_data_.sharedAccess([&](const BlockTreeData &p) {
      // // Register metrics
      // metrics_registry_->registerGaugeFamily(blockHeightMetricName,
      //                                      "Block height info of the chain");
      //
      // metric_best_block_height_ = metrics_registry_->registerGaugeMetric(
      //     blockHeightMetricName, {{"status", "best"}});
      // metric_best_block_height_->set(bestBlockNoLock(p).slot);
      //
      // metric_finalized_block_height_ =
      // metrics_registry_->registerGaugeMetric(
      //     blockHeightMetricName, {{"status", "finalized"}});
      //
      // metric_finalized_block_height_->set(getLastFinalizedNoLock(p).slot);
      //
      // metrics_registry_->registerGaugeFamily(
      //     knownChainLeavesMetricName,
      //     "Number of known chain leaves (aka forks)");
      //
      // metric_known_chain_leaves_ =
      // metrics_registry_->registerGaugeMetric(knownChainLeavesMetricName);
      // metric_known_chain_leaves_->set(p.tree_->leafCount());
      //
      // telemetry_->setGenesisBlockHash(getGenesisBlockHash());
      //
      // if (p.blocks_pruning_.keep_) {
      //   SL_INFO(log_,
      //           "BlocksPruning: enabled with \"--blocks-pruning {}\"",
      //           *p.blocks_pruning_.keep_);
      // }
    });

    // Add non-finalized block to the block tree
    for (auto &item : std::get<1>(initializer->nonFinalizedSubTree())) {
      const auto &block = item.first;
      const auto header = std::move(item.second);

      auto res = BlockTreeImpl::addExistingBlock(block.hash, header);
      if (res.has_error()) {
        SL_WARN(
            log_, "Failed to add existing block {}: {}", block, res.error());
      }
      SL_TRACE(log_,
               "Existing non-finalized block {} is added to block tree",
               block);
    }

    // OUTCOME_TRY(state_pruner->recoverState(*this));
  }

  const BlockHash &BlockTreeImpl::getGenesisBlockHash() const {
    return block_tree_data_
        .sharedAccess([&](const BlockTreeData &p)
                          -> std::reference_wrapper<const BlockHash> {
          if (p.genesis_block_hash_.has_value()) {
            return p.genesis_block_hash_.value();
          }

          auto res = p.storage_->getBlockHash(0);
          BOOST_ASSERT_MSG(res.has_value() and res.value().size() == 1,
                           "Block tree must contain exactly one genesis block");

          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
          const_cast<std::optional<BlockHash> &>(p.genesis_block_hash_)
              .emplace(res.value()[0]);
          return p.genesis_block_hash_.value();
        })
        .get();
  }

  outcome::result<void> BlockTreeImpl::addBlockHeader(
      const BlockHeader &header) {
    return block_tree_data_.exclusiveAccess(
        [&](BlockTreeData &p) -> outcome::result<void> {
          auto parent_opt = p.tree_->find(header.parent);
          if (not parent_opt.has_value()) {
            return BlockTreeError::NO_PARENT;
          }
          const auto &parent = parent_opt.value();
          OUTCOME_TRY(p.storage_->putBlockHeader(header));

          // update local meta with the new block
          auto new_node = std::make_shared<TreeNode>(
              header.index(), parent, isPrimary(header));

          auto reorg = p.tree_->add(new_node);
          OUTCOME_TRY(reorgAndPrune(p, {std::move(reorg), {}}));

          auto header_ptr = std::make_shared<BlockHeader>(header);
          se_manager_->notify(EventTypes::BlockAdded, header_ptr);

          SL_VERBOSE(
              log_, "Block {} has been added into block tree", header.index());

          return outcome::success();
        });
  }

  outcome::result<void> BlockTreeImpl::addBlock(const Block &block) {
    return block_tree_data_.exclusiveAccess(
        [&](BlockTreeData &p) -> outcome::result<void> {
          // Check if we know parent of this block; if not, we cannot insert it
          auto parent_opt = p.tree_->find(block.header.parent);
          if (not parent_opt.has_value()) {
            return BlockTreeError::NO_PARENT;
          }
          const auto &parent = parent_opt.value();

          // Save block
          OUTCOME_TRY(block_hash, p.storage_->putBlock(block));

          // Update local meta with the block
          auto new_node = std::make_shared<TreeNode>(
              block.header.index(), parent, isPrimary(block.header));

          auto reorg = p.tree_->add(new_node);
          OUTCOME_TRY(reorgAndPrune(p, {std::move(reorg), {}}));

          auto header_ptr = std::make_shared<BlockHeader>(block.header);
          se_manager_->notify(EventTypes::BlockAdded, header_ptr);

          SL_DEBUG(log_, "Adding block {}", block_hash);

          // for (const auto &ext : block.extrinsic) {
          // auto extrinsic_hash = p.hasher_->blake2b_256(ext.data);
          // SL_DEBUG(log_, "Adding extrinsic with hash {}", extrinsic_hash);
          // if (auto key = p.extrinsic_event_key_repo_->get(extrinsic_hash))
          // {
          //   main_pool_handler_->execute(
          //       [wself{weak_from_this()}, key{key.value()}, block_hash]() {
          //         if (auto self = wself.lock()) {
          //           self->extrinsic_events_engine_->notify(
          //               key,
          //               primitives::events::ExtrinsicLifecycleEvent::InBlock(
          //                   key, block_hash));
          //         }
          //       });
          // }
          // }

          SL_VERBOSE(log_,
                     "Block {} has been added into block tree",
                     block.header.index());
          return outcome::success();
        });
  }

  void BlockTreeImpl::notifyChainEventsEngine(EventTypes event,
                                              const BlockHeader &header) {
    BOOST_ASSERT(header.hash_opt.has_value());
    auto header_ptr = std::make_shared<BlockHeader>(header);
    se_manager_->notify(event, header_ptr);
  }

  outcome::result<void> BlockTreeImpl::removeLeaf(const BlockHash &block_hash) {
    return block_tree_data_.exclusiveAccess(
        [&](BlockTreeData &p) -> outcome::result<void> {
          auto finalized = getLastFinalizedNoLock(p);
          if (block_hash == finalized.hash) {
            OUTCOME_TRY(header, getBlockHeader(block_hash));
            // OUTCOME_TRY(p.storage_->removeJustification(finalized.hash));

            OUTCOME_TRY(slot, getNumberByHash(header.parent));
            auto parent = BlockIndex(slot, header.parent);

            ReorgAndPrune changes{
                .reorg = Reorg{.common = parent, .revert = {finalized}},
                .prune = {finalized},
            };
            p.tree_ = std::make_unique<CachedTree>(parent);
            OUTCOME_TRY(reorgAndPrune(p, changes));
            return outcome::success();
          }
          if (not p.tree_->isLeaf(block_hash)) {
            return BlockTreeError::BLOCK_IS_NOT_LEAF;
          }
          auto changes = p.tree_->removeLeaf(block_hash);
          OUTCOME_TRY(reorgAndPrune(p, changes));
          return outcome::success();
        });
  }

  // outcome::result<void> BlockTreeImpl::markAsParachainDataBlock(
  //     const BlockHash &block_hash) {
  //   return block_tree_data_.exclusiveAccess(
  //       [&](BlockTreeData &p) -> outcome::result<void> {
  //         SL_TRACE(log_, "Trying to adjust weight for block {}", block_hash);
  //
  //         auto node = p.tree_->find(block_hash);
  //         if (node == nullptr) {
  //           SL_WARN(log_, "Block {} doesn't exists in block tree",
  //           block_hash); return BlockTreeError::BLOCK_NOT_EXISTS;
  //         }
  //
  //         node->contains_approved_para_block = true;
  //         return outcome::success();
  //       });
  // }

  outcome::result<void> BlockTreeImpl::markAsRevertedBlocks(
      const std::vector<BlockHash> &block_hashes) {
    return block_tree_data_.exclusiveAccess(
        [&](BlockTreeData &p) -> outcome::result<void> {
          bool need_to_refresh_best = false;
          auto best = bestBlockNoLock(p);
          for (const auto &block_hash : block_hashes) {
            auto node_opt = p.tree_->find(block_hash);
            if (not node_opt.has_value()) {
              SL_WARN(
                  log_, "Block {} doesn't exists in block tree", block_hash);
              continue;
            }
            auto &node = node_opt.value();

            if (not node->reverted) {
              std::queue<std::shared_ptr<TreeNode>> to_revert;
              to_revert.push(std::move(node));
              while (not to_revert.empty()) {
                auto &reverting_tree_node = to_revert.front();

                reverting_tree_node->reverted = true;

                if (reverting_tree_node->info == best) {
                  need_to_refresh_best = true;
                }

                for (auto &child : reverting_tree_node->children) {
                  if (not child->reverted) {
                    to_revert.push(child);
                  }
                }

                to_revert.pop();
              }
            }
          }
          if (need_to_refresh_best) {
            p.tree_->forceRefreshBest();
          }
          return outcome::success();
        });
  }

  outcome::result<void> BlockTreeImpl::addExistingBlockNoLock(
      BlockTreeData &p,
      const BlockHash &block_hash,
      const BlockHeader &block_header) {
    SL_TRACE(log_,
             "Trying to add block {} into block tree",
             BlockInfo(block_header.slot, block_hash));

    auto node_opt = p.tree_->find(block_hash);
    // Check if the tree doesn't have this block; if not, we skip that
    if (node_opt.has_value()) {
      SL_TRACE(log_,
               "Block {} exists in block tree",
               BlockInfo(block_header.slot, block_hash));
      return BlockTreeError::BLOCK_EXISTS;
    }

    auto parent_opt = p.tree_->find(block_header.parent);

    // Check if we know parent of this block; if not, we cannot insert it
    if (not parent_opt.has_value()) {
      SL_TRACE(log_,
               "Block {} parent of {} has not found in block tree. "
               "Trying to restore missed branch",
               block_header.parent,
               BlockInfo(block_header.slot, block_hash));

      // Trying to restore missed branch
      std::stack<std::pair<BlockHash, BlockHeader>> to_add;

      auto finalized = getLastFinalizedNoLock(p).slot;

      for (auto hash = block_header.parent;;) {
        OUTCOME_TRY(header, p.storage_->getBlockHeader(hash));
        BlockInfo block_index(header.slot, hash);
        SL_TRACE(log_,
                 "Block {} has found in storage and enqueued to add",
                 block_index);

        if (header.slot <= finalized) {
          return BlockTreeError::BLOCK_ON_DEAD_END;
        }

        auto parent_hash = header.parent;
        to_add.emplace(hash, std::move(header));

        if (p.tree_->find(parent_hash).has_value()) {
          SL_TRACE(log_,
                   "Block {} parent of {} has found in block tree",
                   parent_hash,
                   block_index);
          break;
        }

        SL_TRACE(log_,
                 "Block {} has not found in block tree. "
                 "Trying to restore from storage",
                 parent_hash);

        hash = parent_hash;
      }

      while (not to_add.empty()) {
        const auto &[hash, header] = to_add.top();
        OUTCOME_TRY(addExistingBlockNoLock(p, hash, header));
        to_add.pop();
      }

      parent_opt = p.tree_->find(block_header.parent);
      BOOST_ASSERT_MSG(parent_opt.has_value(),
                       "Parent must be restored at this moment");

      SL_TRACE(log_,
               "Trying to add block {} into block tree",
               BlockInfo(block_header.slot, block_hash));
    }
    auto &parent = parent_opt.value();

    // Update local meta with the block
    auto new_node = std::make_shared<TreeNode>(
        block_header.index(), parent, isPrimary(block_header));

    auto reorg = p.tree_->add(new_node);
    OUTCOME_TRY(reorgAndPrune(p, {std::move(reorg), {}}));

    SL_VERBOSE(log_,
               "Block {} has been restored in block tree from storage",
               block_header.index());

    return outcome::success();
  }

  outcome::result<void> BlockTreeImpl::addExistingBlock(
      const BlockHash &block_hash, const BlockHeader &block_header) {
    return block_tree_data_.exclusiveAccess(
        [&](BlockTreeData &p) -> outcome::result<void> {
          return addExistingBlockNoLock(p, block_hash, block_header);
        });
  }

  outcome::result<void> BlockTreeImpl::addBlockBody(const BlockHash &block_hash,
                                                    const BlockBody &body) {
    return block_tree_data_.exclusiveAccess(
        [&](BlockTreeData &p) -> outcome::result<void> {
          return p.storage_->putBlockBody(block_hash, body);
        });
  }

  outcome::result<void> BlockTreeImpl::finalize(
      const BlockHash &block_hash, const Justification &justification) {
    return block_tree_data_.exclusiveAccess([&](BlockTreeData &p)
                                                -> outcome::result<void> {
      auto last_finalized_block_info = getLastFinalizedNoLock(p);
      if (block_hash == last_finalized_block_info.hash) {
        return outcome::success();
      }
      const auto node_opt = p.tree_->find(block_hash);
      if (node_opt.has_value()) {
        auto &node = node_opt.value();

        SL_DEBUG(log_, "Finalizing block {}", node->info);

        OUTCOME_TRY(header, p.storage_->getBlockHeader(block_hash));

        OUTCOME_TRY(p.storage_->putJustification(justification, block_hash));

        std::vector<BlockInfo> retired_hashes;
        for (auto parent = node->parent(); parent; parent = parent->parent()) {
          retired_hashes.emplace_back(parent->info);
        }

        auto changes = p.tree_->finalize(node);
        OUTCOME_TRY(reorgAndPrune(p, changes));
        // OUTCOME_TRY(pruneTrie(p, node->info.slot));

        auto finalized_header_ptr = std::make_shared<BlockHeader>(header);
        se_manager_->notify(EventTypes::BlockFinalized, finalized_header_ptr);

        OUTCOME_TRY(body, p.storage_->getBlockBody(block_hash));
        if (body.has_value()) {
          // for (auto &ext : body.value()) {
          //   auto extrinsic_hash = p.hasher_->blake2b_256(ext.data);
          //   if (auto key = p.extrinsic_event_key_repo_->get(extrinsic_hash))
          //   {
          //     main_pool_handler_->execute([wself{weak_from_this()},
          //                                  key{key.value()},
          //                                  block_hash]() {
          //       if (auto self = wself.lock()) {
          //         self->extrinsic_events_engine_->notify(
          //             key,
          //             primitives::events::ExtrinsicLifecycleEvent::Finalized(
          //                 key, block_hash));
          //       }
          //     });
          //   }
          // }
        }

        struct RemoveAfterFinalizationParams {
          BlockInfo filanized;
          std::vector<BlockInfo> removed;
        };

        auto data_ptr = std::make_shared<RemoveAfterFinalizationParams>(
            header.index(), std::move(retired_hashes));
        se_manager_->notify(EventTypes::DeactivateAfterFinalization, data_ptr);

        log_->info("Finalized block {}", node->info);
        // telemetry_->notifyBlockFinalized(node->info);
        // telemetry_->pushBlockStats();
        // metric_finalized_block_height_->set(node->info.number);

        // we store justification for last finalized block only as long as it is
        // last finalized (if it doesn't meet other justification storage rules,
        // e.g. its number a multiple of 512)
        OUTCOME_TRY(last_finalized_header,
                    p.storage_->getBlockHeader(last_finalized_block_info.hash));
        OUTCOME_TRY(shouldStoreLastFinalized,
                    p.justification_storage_policy_->shouldStoreFor(
                        last_finalized_header, getLastFinalizedNoLock(p).slot));
        if (!shouldStoreLastFinalized) {
          OUTCOME_TRY(
              justification_opt,
              p.storage_->getJustification(last_finalized_block_info.hash));
          if (justification_opt.has_value()) {
            SL_DEBUG(log_,
                     "Purge redundant justification for finalized block {}",
                     last_finalized_block_info);
            OUTCOME_TRY(p.storage_->removeJustification(
                last_finalized_block_info.hash));
          }
        }

        // for (auto end = p.blocks_pruning_.max(node->info.slot);
        //      p.blocks_pruning_.next_ < end;
        //      ++p.blocks_pruning_.next_) {
        //   OUTCOME_TRY(hash,
        //      p.storage_->getBlockHash(p.blocks_pruning_.next_));
        //   if (not hash) {
        //     continue;
        //   }
        //   SL_TRACE(log_,
        //            "BlocksPruning: remove body for block {}",
        //            p.blocks_pruning_.next_);
        //   OUTCOME_TRY(p.storage_->removeBlockBody(*hash));
        // }
      } else {
        OUTCOME_TRY(header, p.storage_->getBlockHeader(block_hash));
        const auto header_number = header.slot;
        if (header_number >= last_finalized_block_info.slot) {
          return BlockTreeError::NON_FINALIZED_BLOCK_NOT_FOUND;
        }

        OUTCOME_TRY(hashes, p.storage_->getBlockHash(header_number));

        if (not qtils::cxx23::ranges::contains(hashes, block_hash)) {
          return BlockTreeError::BLOCK_ON_DEAD_END;
        }

        if (not p.justification_storage_policy_
                    ->shouldStoreFor(header, last_finalized_block_info.slot)
                    .value()) {
          return outcome::success();
        }
        OUTCOME_TRY(justification_opt,
                    p.storage_->getJustification(block_hash));
        if (justification_opt.has_value()) {
          // block already has justification (in DB), fine
          return outcome::success();
        }
        OUTCOME_TRY(p.storage_->putJustification(justification, block_hash));
      }
      return outcome::success();
    });
  }

  // outcome::result<std::optional<BlockHash>>
  // BlockTreeImpl::getBlockHash(BlockNumber block_number) const {
  //   return block_tree_data_.sharedAccess(
  //       [&](const BlockTreeData &p)
  //           -> outcome::result<std::optional<BlockHash>> {
  //         OUTCOME_TRY(hash_opt, p.storage_->getBlockHash(block_number));
  //         return hash_opt;
  //       });
  // }

  bool BlockTreeImpl::has(const BlockHash &hash) const {
    return block_tree_data_.sharedAccess([&](const BlockTreeData &p) {
      return p.tree_->find(hash) or p.storage_->hasBlockHeader(hash).value();
    });
  }

  outcome::result<BlockHeader> BlockTreeImpl::getBlockHeaderNoLock(
      const BlockTreeData &p, const BlockHash &block_hash) const {
    return p.storage_->getBlockHeader(block_hash);
  }

  outcome::result<BlockHeader> BlockTreeImpl::getBlockHeader(
      const BlockHash &block_hash) const {
    return block_tree_data_.sharedAccess(
        [&](const BlockTreeData &p) -> outcome::result<BlockHeader> {
          return getBlockHeaderNoLock(p, block_hash);
        });
  }

  // outcome::result<std::optional<BlockHeader>>
  // BlockTreeImpl::tryGetBlockHeader(
  //     const BlockHash &block_hash) const {
  //   return block_tree_data_.sharedAccess(
  //       [&](const BlockTreeData &p)
  //           -> outcome::result<std::optional<BlockHeader>> {
  //         auto header = p.storage_->getBlockHeader(block_hash);
  //         if (header) {
  //           return header.value();
  //         }
  //         const auto &header_error = header.error();
  //         if (header_error == BlockTreeError::HEADER_NOT_FOUND) {
  //           return std::nullopt;
  //         }
  //         return header_error;
  //       });
  // }

  outcome::result<BlockBody> BlockTreeImpl::getBlockBody(
      const BlockHash &block_hash) const {
    return block_tree_data_.sharedAccess(
        [&](const BlockTreeData &p) -> outcome::result<BlockBody> {
          OUTCOME_TRY(body, p.storage_->getBlockBody(block_hash));
          if (body.has_value()) {
            return body.value();
          }
          return BlockTreeError::BODY_NOT_FOUND;
        });
  }

  // outcome::result<primitives::Justification>
  // BlockTreeImpl::getBlockJustification(
  //     const BlockHash &block_hash) const {
  //   return block_tree_data_.sharedAccess(
  //       [&](const BlockTreeData &p)
  //           -> outcome::result<primitives::Justification> {
  //         OUTCOME_TRY(justification,
  //         p.storage_->getJustification(block_hash)); if
  //         (justification.has_value()) {
  //           return justification.value();
  //         }
  //         return BlockTreeError::JUSTIFICATION_NOT_FOUND;
  //       });
  // }

  outcome::result<std::vector<BlockHash>> BlockTreeImpl::getBestChainFromBlock(
      const BlockHash &block, uint64_t maximum) const {
    return block_tree_data_.sharedAccess(
        [&](const BlockTreeData &p) -> outcome::result<std::vector<BlockHash>> {
          auto block_header_res = p.storage_->getBlockHeader(block);
          if (block_header_res.has_error()) {
            log_->error("cannot retrieve block {}: {}",
                        block,
                        block_header_res.error());
            return BlockTreeError::HEADER_NOT_FOUND;
          }
          auto start_block_number = block_header_res.value().slot;

          if (maximum == 1) {
            return std::vector{block};
          }

          auto current_depth = bestBlockNoLock(p).slot;

          if (start_block_number >= current_depth) {
            return std::vector{block};
          }

          auto count = std::min<uint64_t>(
              current_depth - start_block_number + 1, maximum);

          BlockNumber finish_block_number = start_block_number + count - 1;

          auto finish_block_hash_res =
              p.storage_->getBlockHash(finish_block_number);
          if (finish_block_hash_res.has_error()) {
            log_->error("cannot retrieve block with number {}: {}",
                        finish_block_number,
                        finish_block_hash_res.error());
            return BlockTreeError::HEADER_NOT_FOUND;
          }
          const auto &finish_block_hash = finish_block_hash_res.value()[0];

          OUTCOME_TRY(
              chain,
              getDescendingChainToBlockNoLock(p, finish_block_hash, count));

          if (chain.back() != block) {
            return std::vector{block};
          }
          std::ranges::reverse(chain);
          return chain;
        });
  }

  outcome::result<std::vector<BlockHash>>
  BlockTreeImpl::getDescendingChainToBlockNoLock(const BlockTreeData &p,
                                                 const BlockHash &to_block,
                                                 uint64_t maximum) const {
    std::vector<BlockHash> chain;

    auto hash = to_block;

    // Try to retrieve from cached tree
    if (auto node_opt = p.tree_->find(hash); node_opt.has_value()) {
      auto node = node_opt.value();
      while (maximum > chain.size()) {
        auto parent = node->parent();
        if (not parent) {
          hash = node->info.hash;
          break;
        }
        chain.emplace_back(node->info.hash);
        node = parent;
      }
    }

    while (maximum > chain.size()) {
      auto header_res = p.storage_->getBlockHeader(hash);
      if (header_res.has_error()) {
        if (chain.empty()) {
          log_->error("Cannot retrieve block with hash {}: {}",
                      hash,
                      header_res.error());
          return header_res.error();
        }
        break;
      }
      const auto &header = header_res.value();
      chain.emplace_back(hash);

      if (header.slot == 0) {
        break;
      }
      hash = header.parent;
    }
    return chain;
  }

  outcome::result<std::vector<BlockHash>>
  BlockTreeImpl::getDescendingChainToBlock(const BlockHash &to_block,
                                           uint64_t maximum) const {
    return block_tree_data_.sharedAccess([&](const BlockTreeData &p) {
      return getDescendingChainToBlockNoLock(p, to_block, maximum);
    });
  }

  // BlockTreeImpl::outcome::result<std::vector<BlockHash>>
  // BlockTreeImpl::getChainByBlocks(
  //     const BlockHash &ancestor,
  //     const BlockHash &descendant) const {
  //   return block_tree_data_.sharedAccess(
  //       [&](const BlockTreeData &p) ->
  //       BlockTreeImpl::outcome::result<std::vector<BlockHash>> {
  //         OUTCOME_TRY(from_header, p.storage_->getBlockHeader(ancestor));
  //         auto from = from_header.number;
  //         OUTCOME_TRY(to_header, p.storage_->getBlockHeader(descendant));
  //         auto to = to_header.number;
  //         if (to < from) {
  //           return BlockTreeError::TARGET_IS_PAST_MAX;
  //         }
  //         auto count = to - from + 1;
  //         OUTCOME_TRY(chain,
  //                     getDescendingChainToBlockNoLock(p, descendant, count));
  //         if (chain.size() != count) {
  //           return BlockTreeError::EXISTING_BLOCK_NOT_FOUND;
  //         }
  //         if (chain.back() != ancestor) {
  //           return BlockTreeError::BLOCK_ON_DEAD_END;
  //         }
  //         std::ranges::reverse(chain);
  //         return chain;
  //       });
  // }
  //
  // bool BlockTreeImpl::hasDirectChainNoLock(
  //     const BlockTreeData &p,
  //     const BlockHash &ancestor,
  //     const BlockHash &descendant) const {
  //   if (ancestor == descendant) {
  //     return true;
  //   }
  //   auto ancestor_node_ptr = p.tree_->find(ancestor);
  //   auto descendant_node_ptr = p.tree_->find(descendant);
  //   if (ancestor_node_ptr and descendant_node_ptr) {
  //     return canDescend(descendant_node_ptr, ancestor_node_ptr);
  //   }
  //
  //   /*
  //    * check that ancestor is above descendant
  //    * optimization that prevents reading blockDB up the genesis
  //    * TODO (xDimon) it could be not right place for this check
  //    *  or changing logic may make it obsolete
  //    *  block numbers may be obtained somewhere else
  //    */
  //   BlockNumber ancestor_depth = 0u;
  //   BlockNumber descendant_depth = 0u;
  //   if (ancestor_node_ptr) {
  //     ancestor_depth = ancestor_node_ptr->info.number;
  //   } else {
  //     auto header_res = p.storage_->getBlockHeader(ancestor);
  //     if (!header_res) {
  //       return false;
  //     }
  //     ancestor_depth = header_res.value().number;
  //   }
  //   if (descendant_node_ptr) {
  //     descendant_depth = descendant_node_ptr->info.number;
  //   } else {
  //     auto header_res = p.storage_->getBlockHeader(descendant);
  //     if (!header_res) {
  //       return false;
  //     }
  //     descendant_depth = header_res.value().number;
  //   }
  //   if (descendant_depth < ancestor_depth) {
  //     SL_DEBUG(log_,
  //              "Ancestor block is lower. {} in comparison with {}",
  //              BlockInfo(ancestor_depth, ancestor),
  //              BlockInfo(descendant_depth, descendant));
  //     return false;
  //   }
  //
  //   // Try to use optimal way, if ancestor and descendant in the finalized
  //   // chain
  //   auto finalized = [&](const BlockHash &hash,
  //                        BlockNumber number) {
  //     return number <= getLastFinalizedNoLock(p).number
  //        and p.storage_->getBlockHash(number)
  //                == outcome::success(
  //                    std::optional<BlockHash>(hash));
  //   };
  //   if (descendant_node_ptr or finalized(descendant, descendant_depth)) {
  //     return finalized(ancestor, ancestor_depth);
  //   }
  //
  //   auto current_hash = descendant;
  //   jam_PROFILE_START(search_finalized_chain)
  //   while (current_hash != ancestor) {
  //     auto current_header_res = p.storage_->getBlockHeader(current_hash);
  //     if (!current_header_res) {
  //       return false;
  //     }
  //     if (current_header_res.value().number <= ancestor_depth) {
  //       return false;
  //     }
  //     current_hash = current_header_res.value().parent_hash;
  //   }
  //   jam_PROFILE_END(search_finalized_chain)
  //   return true;
  // }
  //
  // bool BlockTreeImpl::hasDirectChain(
  //     const BlockHash &ancestor,
  //     const BlockHash &descendant) const {
  //   return block_tree_data_.sharedAccess([&](const BlockTreeData &p) {
  //     return hasDirectChainNoLock(p, ancestor, descendant);
  //   });
  // }
  //
  // bool BlockTreeImpl::isFinalized(const BlockInfo &block) const {
  //   return block_tree_data_.sharedAccess([&](const BlockTreeData &p) {
  //     return block.number <= getLastFinalizedNoLock(p).number
  //        and p.storage_->getBlockHash(block.number)
  //                == outcome::success(
  //                    std::optional<BlockHash>(block.hash));
  //   });
  // }

  BlockInfo BlockTreeImpl::bestBlockNoLock(const BlockTreeData &p) const {
    return p.tree_->best();
  }

  BlockInfo BlockTreeImpl::bestBlock() const {
    return block_tree_data_.sharedAccess(
        [&](const BlockTreeData &p) { return bestBlockNoLock(p); });
  }

  outcome::result<BlockInfo> BlockTreeImpl::getBestContaining(
      const BlockHash &target_hash) const {
    return block_tree_data_.sharedAccess(
        [&](const BlockTreeData &p) -> outcome::result<BlockInfo> {
          if (getLastFinalizedNoLock(p).hash == target_hash) {
            return bestBlockNoLock(p);
          }

          auto target_node_opt = p.tree_->find(target_hash);

          // If a target has not found in the block tree (in memory),
          // it means block finalized or discarded
          if (not target_node_opt.has_value()) {
            OUTCOME_TRY(target_header, p.storage_->getBlockHeader(target_hash));
            auto target_number = target_header.slot;

            OUTCOME_TRY(hashes, p.storage_->getBlockHash(target_number));

            if (not qtils::cxx23::ranges::contains(hashes, target_hash)) {
              return BlockTreeError::BLOCK_ON_DEAD_END;
            }

            return bestBlockNoLock(p);
          }

          return p.tree_->bestWith(target_node_opt.value());
        });
  }

  std::vector<BlockHash> BlockTreeImpl::getLeavesNoLock(
      const BlockTreeData &p) const {
    return p.tree_->leafHashes();
  }

  std::vector<BlockHash> BlockTreeImpl::getLeaves() const {
    return block_tree_data_.sharedAccess(
        [&](const BlockTreeData &p) { return getLeavesNoLock(p); });
  }

  // std::vector<BlockInfo> BlockTreeImpl::getLeavesInfo() const {
  //   return block_tree_data_.sharedAccess(
  //       [&](const BlockTreeData &p) { return p.tree_->leafInfo(); });
  // }

  outcome::result<std::vector<BlockHash>> BlockTreeImpl::getChildren(
      const BlockHash &block) const {
    return block_tree_data_.sharedAccess(
        [&](const BlockTreeData &p) -> outcome::result<std::vector<BlockHash>> {
          if (auto node_opt = p.tree_->find(block); node_opt.has_value()) {
            const auto &node = node_opt.value();
            std::vector<BlockHash> result;
            result.reserve(node->children.size());
            for (const auto &child : node->children) {
              result.push_back(child->info.hash);
            }
            return result;
          }
          OUTCOME_TRY(header, p.storage_->getBlockHeader(block));

          // TODO slot of children can be greater then parent's by more then one
          return p.storage_->getBlockHash(header.slot + 1);
        });
  }

  BlockInfo BlockTreeImpl::getLastFinalizedNoLock(
      const BlockTreeData &p) const {
    return p.tree_->finalized();
  }

  BlockInfo BlockTreeImpl::getLastFinalized() const {
    return block_tree_data_.sharedAccess(
        [&](const BlockTreeData &p) { return getLastFinalizedNoLock(p); });
  }

  outcome::result<void> BlockTreeImpl::reorgAndPrune(
      BlockTreeData &p, const ReorgAndPrune &changes) {
    OUTCOME_TRY(p.storage_->setBlockTreeLeaves(p.tree_->leafHashes()));
    // metric_known_chain_leaves_->set(p.tree_->leafCount());
    if (changes.reorg) {
      for (auto &block : changes.reorg->revert) {
        OUTCOME_TRY(p.storage_->deassignHashToSlot(block));
      }
      for (auto &block : changes.reorg->apply) {
        OUTCOME_TRY(p.storage_->assignHashToSlot(block));
      }
      // if (not changes.reorg->apply.empty()) {
      //   metric_best_block_height_->set(changes.reorg->apply.back().number);
      // } else {
      //   metric_best_block_height_->set(changes.reorg->common.number);
      // }
    }

    // std::vector<Extrinsic> extrinsics;

    // remove from storage
    for (const auto &block : changes.prune) {
      OUTCOME_TRY(block_header, p.storage_->getBlockHeader(block.hash));
      OUTCOME_TRY(block_body_opt, p.storage_->getBlockBody(block.hash));
      if (block_body_opt.has_value()) {
        // extrinsics.reserve(extrinsics.size() +
        // block_body_opt.value().size()); for (auto &ext :
        // block_body_opt.value()) {
        //   auto extrinsic_hash = p.hasher_->blake2b_256(ext.data);
        //   if (auto key = p.extrinsic_event_key_repo_->get(extrinsic_hash)) {
        //     main_pool_handler_->execute([wself{weak_from_this()},
        //                                  key{key.value()},
        //                                  block_hash{block.hash}]() {
        //       if (auto self = wself.lock()) {
        //         self->extrinsic_events_engine_->notify(
        //             key,
        //             primitives::events::ExtrinsicLifecycleEvent::Retracted(
        //                 key, block_hash));
        //       }
        //     });
        //   }
        //   extrinsics.emplace_back(std::move(ext));
        // }
        // p.state_pruner_->schedulePrune(
        //     block_header.state_root,
        //     block_header.index(),
        //     storage::trie_pruner::PruneReason::Discarded);
      }
      OUTCOME_TRY(p.storage_->removeBlock(block.hash));
    }

    return outcome::success();
  }

  // outcome::result<void> BlockTreeImpl::pruneTrie(
  //     const BlockTreeData &block_tree_data,
  //     BlockNumber new_finalized) {
  //   // pruning is disabled
  //   if (!block_tree_data.state_pruner_->getPruningDepth().has_value()) {
  //     return outcome::success();
  //   }
  //   auto last_pruned = block_tree_data.state_pruner_->getLastPrunedBlock();
  //
  //   BOOST_ASSERT(!last_pruned.has_value()
  //                || last_pruned.value().number
  //                       <= getLastFinalizedNoLock(block_tree_data).number);
  //   auto next_pruned_number = last_pruned ? last_pruned->number + 1 : 0;
  //
  //   OUTCOME_TRY(hash_opt, getBlockHash(next_pruned_number));
  //   BOOST_ASSERT(hash_opt.has_value());
  //   auto &hash = hash_opt.value();
  //   auto pruning_depth =
  //       block_tree_data.state_pruner_->getPruningDepth().value_or(0);
  //   if (new_finalized < pruning_depth) {
  //     return outcome::success();
  //   }
  //
  //   auto last_to_prune = new_finalized - pruning_depth;
  //   for (auto n = next_pruned_number; n < last_to_prune; n++) {
  //     OUTCOME_TRY(next_hash_opt, getBlockHash(n + 1));
  //     BOOST_ASSERT(next_hash_opt.has_value());
  //     auto &next_hash = *next_hash_opt;
  //     OUTCOME_TRY(header, getBlockHeader(hash));
  //     block_tree_data.state_pruner_->schedulePrune(
  //         header.state_root,
  //         header.index(),
  //         storage::trie_pruner::PruneReason::Finalized);
  //     hash = next_hash;
  //   }
  //
  //   return outcome::success();
  // }
  //
  // void BlockTreeImpl::warp(const BlockInfo &block_info) {
  //   block_tree_data_.exclusiveAccess([&](BlockTreeData &p) {
  //     p.tree_ = std::make_unique<CachedTree>(block_info);
  //     metric_known_chain_leaves_->set(1);
  //     metric_best_block_height_->set(block_info.number);
  //     telemetry_->notifyBlockFinalized(block_info);
  //     telemetry_->pushBlockStats();
  //     metric_finalized_block_height_->set(block_info.number);
  //   });
  // }

  void BlockTreeImpl::notifyBestAndFinalized() {
    auto best_info = bestBlock();
    auto best_header = getBlockHeader(best_info.hash).value();

    auto best_header_ptr = std::make_shared<BlockHeader>(best_header);
    se_manager_->notify(EventTypes::BlockAdded, best_header_ptr);

    auto finalized_info = getLastFinalized();
    auto finalized_header = getBlockHeader(finalized_info.hash).value();

    auto finalized_header_ptr = std::make_shared<BlockHeader>(finalized_header);
    se_manager_->notify(EventTypes::BlockFinalized, finalized_header_ptr);
  }

  void BlockTreeImpl::removeUnfinalized() {
    block_tree_data_.exclusiveAccess([&](BlockTreeData &p) {
      auto changes = p.tree_->removeUnfinalized();
      if (auto r = reorgAndPrune(p, changes); r.has_error()) {
        SL_WARN(log_, "removeUnfinalized error: {}", r.error());
      }
    });
  }

  // BlockHeaderRepository methods

  outcome::result<BlockNumber> BlockTreeImpl::getNumberByHash(
      const BlockHash &hash) const {
    auto slot_opt = block_tree_data_.sharedAccess(
        [&](const BlockTreeData &p) -> std::optional<BlockNumber> {
          if (auto node = p.tree_->find(hash)) {
            return node.value()->info.slot;
          }
          return std::nullopt;
        });
    if (slot_opt.has_value()) {
      return slot_opt.value();
    }
    OUTCOME_TRY(header, getBlockHeader(hash));
    return header.slot;
  }

  // outcome::result<BlockHash> BlockTreeImpl::getHashByNumber(
  //     BlockNumber number) const {
  //   OUTCOME_TRY(block_hash_opt, getBlockHash(number));
  //   if (block_hash_opt.has_value()) {
  //     return block_hash_opt.value();
  //   }
  //   return BlockTreeError::HEADER_NOT_FOUND;
  // }
  //
  // BlockTreeImpl::BlocksPruning::BlocksPruning(std::optional<uint32_t> keep,
  //                                             BlockNumber
  //                                             finalized)
  //     : keep_{keep}, next_{max(finalized)} {}
  //
  // BlockNumber BlockTreeImpl::BlocksPruning::max(
  //     BlockNumber finalized) const {
  //   return keep_ and finalized > *keep_ ? finalized - *keep_ : 0;
  // }

}  // namespace jam::blockchain
