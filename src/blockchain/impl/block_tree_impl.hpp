/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "blockchain/block_tree.hpp"

// #include <functional>
// #include <memory>
// #include <optional>
// #include <queue>
// #include <thread>
// #include <unordered_set>

#include <qtils/final_action.hpp>

#include "app/configuration.hpp"
#include "blockchain/block_storage.hpp"
#include "blockchain/impl/block_tree_initializer.hpp"
// #include "blockchain/block_tree_error.hpp"
#include "blockchain/impl/cached_tree.hpp"
// #include "consensus/babe/types/babe_configuration.hpp"
// #include "consensus/timeline/types.hpp"
// #include "crypto/hasher.hpp"
#include <thread>

#include "log/logger.hpp"
#include "se/impl/common.hpp"
#include "se/subscription_fwd.hpp"
// #include "metrics/metrics.hpp"
// #include "primitives/event_types.hpp"
// #include "storage/trie/trie_storage.hpp"
// #include "subscription/extrinsic_event_key_repository.hpp"
// #include "telemetry/service.hpp"
// #include "utils/safe_object.hpp"

namespace jam::app {
  class Configuration;
}

// namespace jam {
//   class PoolHandler;
// }  // namespace jam
//
namespace jam::blockchain {
  //   struct ReorgAndPrune;
  //   class TreeNode;
  class BlockTreeInitializer;
}  // namespace jam::blockchain
//
// namespace jam::common {
//   class MainThreadPool;
// }
//
// namespace jam::storage::trie_pruner {
//   class TriePruner;
// }

namespace jam::blockchain {

  class BlockTreeImpl : public BlockTree,
                        public std::enable_shared_from_this<BlockTreeImpl> {
   public:
    // /// Recover block tree state at provided block
    // static outcome::result<void> recover(
    //     const BlockId &target_block_id,
    //     std::shared_ptr<BlockStorage> storage,
    //     std::shared_ptr<const storage::trie::TrieStorage> trie_storage,
    //     std::shared_ptr<blockchain::BlockTree> block_tree);

    BlockTreeImpl(
        qtils::SharedRef<log::LoggingSystem> logsys,
        qtils::SharedRef<const app::Configuration> app_config,
        qtils::SharedRef<BlockStorage> storage,
        // // const BlockInfo &finalized,
        qtils::SharedRef<crypto::Hasher> hasher,
        // primitives::events::ChainSubscriptionEnginePtr chain_events_engine,
        // primitives::events::ExtrinsicSubscriptionEnginePtr
        //     extrinsic_events_engine,
        // std::shared_ptr<subscription::ExtrinsicEventKeyRepository>
        //     extrinsic_event_key_repo,
        std::shared_ptr<const class JustificationStoragePolicy>
            justification_storage_policy,
        // std::shared_ptr<storage::trie_pruner::TriePruner> state_pruner,
        // common::MainThreadPool &main_thread_pool
        qtils::SharedRef<BlockTreeInitializer> initializer);

    ~BlockTreeImpl() override = default;

    const BlockHash &getGenesisBlockHash() const override;

    // outcome::result<std::optional<BlockHash>> getBlockHash(
    //     BlockNumber block_number) const override;

    bool has(const BlockHash &hash) const override;

    outcome::result<BlockHeader> getBlockHeader(
        const BlockHash &block_hash) const override;

    //   outcome::result<std::optional<BlockHeader>> tryGetBlockHeader(
    //       const BlockHash &block_hash) const override;

    outcome::result<BlockBody> getBlockBody(
        const BlockHash &block_hash) const override;

    // outcome::result<Justification> getBlockJustification(
    //       const BlockHash &block_hash) const override;

    outcome::result<void> addBlockHeader(const BlockHeader &header) override;

    outcome::result<void> addBlock(const Block &block) override;

    outcome::result<void> removeLeaf(const BlockHash &block_hash) override;

    outcome::result<void> addExistingBlock(
        const BlockHash &block_hash, const BlockHeader &block_header) override;

    // outcome::result<void> markAsParachainDataBlock(
    //     const BlockHash &block_hash) override;

    outcome::result<void> markAsRevertedBlocks(
        const std::vector<BlockHash> &block_hashes) override;

    outcome::result<void> addBlockBody(const BlockHash &block_hash,
                                       const BlockBody &body) override;

    outcome::result<void> finalize(const BlockHash &block_hash,
                                   const Justification &justification) override;

    outcome::result<std::vector<BlockHash>> getBestChainFromBlock(
        const BlockHash &block, uint64_t maximum) const override;

    outcome::result<std::vector<BlockHash>> getDescendingChainToBlock(
        const BlockHash &block, uint64_t maximum) const override;

    // outcome::result<std::vector<BlockHash>> getChainByBlocks(
    //     const BlockHash &ancestor,
    //     const BlockHash &descendant) const override;
    //
    // bool hasDirectChain(const BlockHash &ancestor,
    //                     const BlockHash &descendant) const override;
    //
    // bool isFinalized(const BlockInfo &block) const override;

    BlockInfo bestBlock() const override;

    outcome::result<BlockInfo> getBestContaining(
        const BlockHash &target_hash) const override;

    std::vector<BlockHash> getLeaves() const override;
    // std::vector<BlockInfo> getLeavesInfo() const override;

    outcome::result<std::vector<BlockHash>> getChildren(
        const BlockHash &block) const override;

    BlockInfo getLastFinalized() const override;

    // void warp(const BlockInfo &block_info) override;

    void notifyBestAndFinalized() override;

    void removeUnfinalized() override;

    // BlockHeaderRepository methods

    outcome::result<BlockNumber> getNumberByHash(
        const BlockHash &block_hash) const override;

    // outcome::result<BlockHash> getHashByNumber(
    //     BlockNumber block_number) const override;

   private:
    // struct BlocksPruning {
    //   BlocksPruning(std::optional<uint32_t> keep,
    //                 BlockNumber finalized);
    //
    //   BlockNumber max(BlockNumber finalized) const;
    //
    //   std::optional<uint32_t> keep_;
    //   BlockNumber next_;
    // };

    struct BlockTreeData {
      qtils::SharedRef<BlockStorage> storage_;
      // std::shared_ptr<storage::trie_pruner::TriePruner> state_pruner_;
      std::unique_ptr<CachedTree> tree_;
      qtils::SharedRef<crypto::Hasher> hasher_;
      std::shared_ptr<const class JustificationStoragePolicy>
          justification_storage_policy_;
      std::optional<BlockHash> genesis_block_hash_;
      // BlocksPruning blocks_pruning_;
    };

    outcome::result<void> reorgAndPrune(BlockTreeData &p,
                                        const ReorgAndPrune &changes);

    outcome::result<BlockHeader> getBlockHeaderNoLock(
        const BlockTreeData &p, const BlockHash &block_hash) const;

    //   outcome::result<void> pruneTrie(const BlockTreeData &block_tree_data,
    //                                   BlockNumber new_finalized);

    BlockInfo getLastFinalizedNoLock(const BlockTreeData &p) const;
    BlockInfo bestBlockNoLock(const BlockTreeData &p) const;

    // bool hasDirectChainNoLock(const BlockTreeData &p,
    //                           const BlockHash &ancestor,
    //                           const BlockHash &descendant);
    std::vector<BlockHash> getLeavesNoLock(const BlockTreeData &p) const;

    outcome::result<std::vector<BlockHash>> getDescendingChainToBlockNoLock(
        const BlockTreeData &p,
        const BlockHash &to_block,
        uint64_t maximum) const;

    outcome::result<void> addExistingBlockNoLock(
        BlockTreeData &p,
        const BlockHash &block_hash,
        const BlockHeader &block_header);

    void notifyChainEventsEngine(EventTypes event, const BlockHeader &header);

    class SafeBlockTreeData {
     public:
      SafeBlockTreeData(BlockTreeData data);

      template <typename F>
      decltype(auto) exclusiveAccess(F &&f) {
        // if this thread owns the mutex, it shall
        // not be unlocked until this function exits
        if (exclusive_owner_.load(std::memory_order_acquire)
            == std::this_thread::get_id()) {
          return f(block_tree_data_.unsafeGet());
        }
        return block_tree_data_.exclusiveAccess([&f,
                                                 this](BlockTreeData &data) {
          exclusive_owner_ = std::this_thread::get_id();
          qtils::FinalAction reset([&] { exclusive_owner_ = std::nullopt; });
          return f(data);
        });
      }

      template <typename F>
      decltype(auto) sharedAccess(F &&f) const {
        // if this thread owns the mutex, it shall
        // not be unlocked until this function exits
        if (exclusive_owner_.load(std::memory_order_acquire)
            == std::this_thread::get_id()) {
          return f(block_tree_data_.unsafeGet());
        }
        return block_tree_data_.sharedAccess(std::forward<F>(f));
      }

     private:
      se::utils::SafeObject<BlockTreeData> block_tree_data_;
      std::atomic<std::optional<std::thread::id>> exclusive_owner_;
    };

    log::Logger log_;

    SafeBlockTreeData block_tree_data_;

    // primitives::events::ChainSubscriptionEnginePtr chain_events_engine_;
    // std::shared_ptr<PoolHandler> main_pool_handler_;
    // primitives::events::ExtrinsicSubscriptionEnginePtr
    // extrinsic_events_engine_;

    // // Metrics
    // metrics::RegistryPtr metrics_registry_ = metrics::createRegistry();
    // metrics::Gauge *metric_best_block_height_;
    // metrics::Gauge *metric_finalized_block_height_;
    // metrics::Gauge *metric_known_chain_leaves_;
    // telemetry::Telemetry telemetry_ = telemetry::createTelemetryService();
  };

}  // namespace jam::blockchain
