/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <qtils/cxx23/ranges/contains.hpp>
#include <qtils/test/outcome.hpp>

#include "app/configuration.hpp"
#include "blockchain/block_tree_error.hpp"
#include "blockchain/impl/block_tree_impl.hpp"
#include "blockchain/impl/cached_tree.hpp"
// #include "common/main_thread_pool.hpp"
#include "blockchain/impl/block_storage_initializer.hpp"
// #include "consensus/babe/types/babe_block_header.hpp"
// #include "consensus/babe/types/seal.hpp"
#include "crypto/hasher/hasher_impl.hpp"
// #include "mock/core/app/configuration_mock.hpp"
#include "mock/app/state_manager_mock.hpp"
#include "mock/blockchain/block_storage_mock.hpp"
#include "mock/blockchain/justification_storage_policy_mock.hpp"
// #include "mock/core/consensus/babe/babe_config_repository_mock.hpp"
// #include "mock/core/storage/trie_pruner/trie_pruner_mock.hpp"
// #include "mock/core/transaction_pool/transaction_pool_mock.hpp"
// #include "network/impl/extrinsic_observer_impl.hpp"
// #include "scale/kagome_scale.hpp"
#include <se/impl/sync_dispatcher_impl.hpp>

#include "jam_types/justification.hpp"
#include "se/subscription.hpp"
#include "testutil/literals.hpp"
#include "testutil/prepare_loggers.hpp"

using jam::Block;
using jam::BlockBody;
using jam::BlockHash;
using jam::BlockHeader;
using jam::BlockId;
using jam::BlockInfo;
using jam::BlockNumber;
using jam::calculateBlockHash;
using jam::encode;
using jam::TimeSlot;
using jam::app::Configuration;
using jam::app::StateManagerMock;
using jam::blockchain::BlockStorageMock;
using jam::blockchain::BlockTreeError;
using jam::blockchain::BlockTreeImpl;
using jam::blockchain::BlockTreeInitializer;
using jam::blockchain::JustificationStoragePolicyMock;
using jam::blockchain::TreeNode;
// using jam::consensus::babe::BabeBlockHeader;
// using jam::consensus::babe::SlotType;
using jam::crypto::HasherImpl;
// using jam::network::ExtrinsicObserverImpl;
// using jam::primitives::Consensus;
// using jam::primitives::Digest;
using jam::Justification;
// using jam::primitives::PreRuntime;
// using jam::storage::trie_pruner::TriePrunerMock;
// using jam::transaction_pool::TransactionPoolMock;
using qtils::ByteVec;
using qtils::literals::operator""_vec;
// using BabeSeal = consensus::babe::Seal;
// using Seal = primitives::Seal;
using RootHash = jam::OpaqueHash;

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::ReturnRef;
using testing::StrictMock;

namespace kagome::primitives {
  void PrintTo(const BlockHeader &header, std::ostream *os) {
    *os << "BlockHeader {\n"
        << "\tslot: " << header.slot << ",\n"
        << "\tparent: " << header.parent << ",\n"
        << "\tparent_state_root: " << header.parent_state_root << ",\n"
        << "\textrinsic_hash: " << header.extrinsic_hash << "\n}";
  }
}  // namespace kagome::primitives

struct BlockTreeTest : testing::Test {
  static void SetUpTestCase() {
    testutil::prepareLoggers();
  }
  static void TearDownTestCase() {}

  void SetUp() override {
    auto dispatcher = std::make_shared<jam::se::SyncDispatcher<1, 1>>();
    se_ = std::make_shared<jam::Subscription>(dispatcher);

    EXPECT_CALL(*storage_, getBlockTreeLeaves())
        .WillOnce(Return(std::vector{kFinalizedBlockInfo.hash}));

    EXPECT_CALL(*storage_, setBlockTreeLeaves(_))
        .WillRepeatedly(Return(outcome::success()));

    for (BlockNumber i = 1; i < 100; ++i) {
      EXPECT_CALL(*storage_, getBlockHash(i))
          .WillRepeatedly(Return(std::vector{kFirstBlockInfo.hash}));
    }

    EXPECT_CALL(*storage_, hasBlockHeader(kFirstBlockInfo.hash))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*storage_, getBlockHeader(kFirstBlockInfo.hash))
        .WillRepeatedly(Return(first_block_header_));

    EXPECT_CALL(*storage_, getBlockHeader(kFinalizedBlockInfo.hash))
        .WillRepeatedly(Return(finalized_block_header_));

    EXPECT_CALL(*storage_, getJustification(kFinalizedBlockInfo.hash))
        .WillRepeatedly(Return(outcome::success(Justification{})));

    EXPECT_CALL(*storage_, getLastFinalized())
        .WillOnce(Return(outcome::success(kFinalizedBlockInfo)));

    EXPECT_CALL(*storage_, removeBlock(_))
        .WillRepeatedly(Invoke([&](const auto &hash) {
          delSlotToHash(hash);
          return outcome::success();
        }));

    EXPECT_CALL(*storage_, getBlockHash(testing::Matcher<BlockNumber>(_)))
        .WillRepeatedly(Invoke(
            [&](BlockNumber n) -> outcome::result<std::vector<BlockHash>> {
              auto it = slot_to_hash_.find(n);
              if (it == slot_to_hash_.end()) {
                return BlockTreeError::HEADER_NOT_FOUND;
              }
              return std::vector{it->second};
            }));

    EXPECT_CALL(*storage_, getBlockHeader({finalized_block_header_.parent}))
        .WillRepeatedly(Return(BlockTreeError::HEADER_NOT_FOUND));

    EXPECT_CALL(*storage_, getBlockHeader(kFinalizedBlockInfo.hash))
        .WillRepeatedly(Return(finalized_block_header_));

    EXPECT_CALL(*storage_, assignHashToSlot(_))
        .WillRepeatedly(
            Invoke([&](const BlockInfo &b) -> outcome::result<void> {
              putSlotToHash(b);
              return outcome::success();
            }));

    EXPECT_CALL(*storage_, deassignHashToSlot(_))
        .WillRepeatedly(
            Invoke([&](const BlockInfo &b) -> outcome::result<void> {
              delSlotToHash(b);
              return outcome::success();
            }));

    // ON_CALL(*state_pruner_, recoverState(_))
    //     .WillByDefault(Return(outcome::success()));

    // ON_CALL(*state_pruner_, schedulePrune(_, _, _)).WillByDefault(Return());

    putSlotToHash(kGenesisBlockInfo);
    putSlotToHash(kFinalizedBlockInfo);

    auto logsys = testutil::prepareLoggers();

    // Initialize the block tree
    qtils::SharedRef initializer =
        std::make_shared<BlockTreeInitializer>(logsys, storage_);

    block_tree_ = std::make_shared<BlockTreeImpl>(logsys,
                                                  app_config_,
                                                  storage_,
                                                  hasher_,
                                                  justification_storage_policy_,
                                                  se_,
                                                  initializer);
  }

  void TearDown() override {
    // watchdog_->stop();
    se_->dispose();
    se_.reset();
  }

  /**
   * Add a block with some data, which is a child of the top-most block
   * @return block, which was added, along with its hash
   */
  BlockHash addBlock(const Block &block) {
    auto encoded_block = encode(block).value();
    auto hash = hasher_->blake2b_256(encoded_block);
    BlockInfo block_info(block.header.slot, hash);
    const_cast<BlockHeader &>(block.header).hash_opt.emplace(hash);

    EXPECT_CALL(*storage_, putBlock(block))
        .WillRepeatedly(Invoke([&](const auto &block) {
          putSlotToHash(block_info);
          return hash;
        }));

    // for reorganizing
    EXPECT_CALL(*storage_, getBlockHeader(hash))
        .WillRepeatedly(Return(block.header));

    EXPECT_TRUE(block_tree_->addBlock(block));

    return hash;
  }

  /**
   * Creates block and add it to block tree
   * @param parent - hash of parent block
   * @param slot - slot of new block
   * @param state - hash of parent state root
   * @param is_primary - true if block created by ticket winning
   * @return hash newly created block
   * @note To create different block with same slot and parent, use different
   * hash ot state root
   */
  std::tuple<BlockHash, BlockHeader> addHeaderToRepositoryAndGet(
      const BlockHash &parent, TimeSlot slot, RootHash state, bool is_primary) {
    BlockHeader header;
    header.parent = parent;
    header.slot = slot;
    header.parent_state_root = state;

    // Trick with fake extrinsic hash to mark a block producing way
    header.extrinsic_hash = is_primary ? "ticket"_arr32 : "fallback"_arr32;

    calculateBlockHash(header, *hasher_);

    auto hash = addBlock(Block{header, {}});

    // hash for header repo and number for block storage just because tests
    // currently require so
    EXPECT_CALL(*storage_, getBlockHeader(hash)).WillRepeatedly(Return(header));

    return {hash, header};
  }

  uint32_t state_nonce_ = 0;
  BlockHash addHeaderToRepository(const BlockHash &parent, BlockNumber number) {
    RootHash state;
    memcpy(state.data(), &state_nonce_, sizeof(state_nonce_));
    ++state_nonce_;
    return std::get<0>(
        addHeaderToRepositoryAndGet(parent, number, state, false));
  }

  BlockHash addHeaderToRepository(const BlockHash &parent,
                                  BlockNumber number,
                                  bool is_primary) {
    return std::get<0>(
        addHeaderToRepositoryAndGet(parent, number, {}, is_primary));
  }

  BlockHash addHeaderToRepository(const BlockHash &parent,
                                  BlockNumber number,
                                  RootHash state) {
    return std::get<0>(
        addHeaderToRepositoryAndGet(parent, number, state, false));
  }

  std::shared_ptr<jam::Subscription> se_;

  std::shared_ptr<Configuration> app_config_ =
      std::make_shared<Configuration>();

  std::shared_ptr<BlockStorageMock> storage_ =
      std::make_shared<BlockStorageMock>();

  // std::shared_ptr<TransactionPoolMock> pool_ =
  //     std::make_shared<TransactionPoolMock>();

  // std::shared_ptr<ExtrinsicObserverImpl> extrinsic_observer_ =
  //     std::make_shared<ExtrinsicObserverImpl>(pool_);

  std::shared_ptr<HasherImpl> hasher_ = std::make_shared<HasherImpl>();

  std::shared_ptr<JustificationStoragePolicyMock>
      justification_storage_policy_ =
          std::make_shared<StrictMock<JustificationStoragePolicyMock>>();

  // std::shared_ptr<TriePrunerMock> state_pruner_ =
  //     std::make_shared<TriePrunerMock>();

  std::shared_ptr<StateManagerMock> app_state_manager_ =
      std::make_shared<StateManagerMock>();

  std::shared_ptr<BlockTreeImpl> block_tree_;

  const BlockId kLastFinalizedBlockId = kFinalizedBlockInfo.hash;

  // static Digest make_digest(SlotNumber slot,
  //                           SlotType slot_type = SlotType::SecondaryPlain) {
  //   Digest digest;
  //
  //   BabeBlockHeader babe_header{
  //       .slot_assignment_type = slot_type,
  //       .authority_index = 0,
  //       .slot_number = slot,
  //   };
  //   ByteVec encoded_header{encode(babe_header).value()};
  //   digest.emplace_back(
  //       PreRuntime{{primitives::kBabeEngineId, encoded_header}});
  //
  //   BabeSeal seal{};
  //   ByteVec encoded_seal{encode(seal).value()};
  //   digest.emplace_back(Seal{{primitives::kBabeEngineId, encoded_seal}});
  //
  //   return digest;
  // }

  const BlockInfo kGenesisBlockInfo{
      0ul, BlockHash::fromString("genesis_block___________________").value()};

  BlockHeader first_block_header_{.parent = kGenesisBlockInfo.hash,
                                  .slot = 1,
                                  .hash_opt = kGenesisBlockInfo.hash};

  const BlockInfo kFirstBlockInfo{
      1ul, BlockHash::fromString("first_block_____________________").value()};

  const BlockInfo kFinalizedBlockInfo{
      42ull, BlockHash::fromString("finalized_block_________________").value()};

  BlockHeader finalized_block_header_{
      .parent =
          BlockHash::fromString("parent_of_finalized_____________").value(),
      .slot = kFinalizedBlockInfo.slot,
      .hash_opt = kFinalizedBlockInfo.hash};

  BlockBody finalized_block_body_{
      //{ByteVec{0x22, 0x44}}, {ByteVec{0x55, 0x66}}
  };

  std::map<TimeSlot, std::vector<BlockHash>> slot_to_hash_;

  void putSlotToHash(const BlockInfo &b) {
    auto it = slot_to_hash_.find(b.slot);
    if (it == slot_to_hash_.end()) {
      slot_to_hash_.emplace(b.slot, std::vector{b.hash});
    } else {
      auto &hashes = it->second;
      if (not qtils::cxx23::ranges::contains(hashes, b.hash)) {
        hashes.emplace_back(b.hash);
      }
    }
  }
  void delSlotToHash(const BlockInfo &b) {
    auto it = slot_to_hash_.find(b.slot);
    if (it == slot_to_hash_.end()) {
      return;
      ;
    }
    auto &hashes = it->second;
    auto to_erase = std::ranges::remove(hashes, b.hash);
    if (not to_erase.empty()) {
      hashes.erase(to_erase.begin(), to_erase.end());
      if (hashes.empty()) {
        slot_to_hash_.erase(it);
      }
    }
  }
  void delSlotToHash(const BlockHash &hash) {
    for (auto it = slot_to_hash_.begin(); it != slot_to_hash_.end();) {
      auto &hashes = it->second;
      auto to_erase = std::ranges::remove(hashes, hash);
      if (not to_erase.empty()) {
        hashes.erase(to_erase.begin(), to_erase.end());
        if (hashes.empty()) {
          it = slot_to_hash_.erase(it);
          continue;
        }
      }
      ++it;
    }
  }

  BlockHeader makeBlockHeader(TimeSlot slot, BlockHash parent) const {
    BlockHeader header{.parent = parent, .slot = slot};
    calculateBlockHash(header, *hasher_);
    return header;
  }
};

/**
 * @given block tree with at least one block inside
 * @when requesting body of that block
 * @then body is returned
 */
TEST_F(BlockTreeTest, GetBody) {
  // GIVEN
  // WHEN
  EXPECT_CALL(*storage_, getBlockBody(kFinalizedBlockInfo.hash))
      .WillOnce(Return(finalized_block_body_));

  // THEN
  ASSERT_OUTCOME_SUCCESS(body,
                         block_tree_->getBlockBody(kFinalizedBlockInfo.hash));
  ASSERT_EQ(body, finalized_block_body_);
}

/**
 * @given block tree with at least one block inside
 * @when adding a new block, which is a child of that block.
 * @then block is added
 */
TEST_F(BlockTreeTest, AddBlock) {
  // GIVEN
  auto &&[deepest_block_number, deepest_block_hash] = block_tree_->bestBlock();
  ASSERT_EQ(deepest_block_hash, kFinalizedBlockInfo.hash);

  auto leaves = block_tree_->getLeaves();
  ASSERT_EQ(leaves.size(), 1);
  ASSERT_EQ(leaves[0], kFinalizedBlockInfo.hash);

  auto children_res = block_tree_->getChildren(kFinalizedBlockInfo.hash);
  ASSERT_TRUE(children_res);
  ASSERT_TRUE(children_res.value().empty());

  // WHEN
  Block new_block{.header = makeBlockHeader(kFinalizedBlockInfo.slot + 1,
                                            kFinalizedBlockInfo.hash)};
  auto hash = addBlock(new_block);

  // THEN
  auto new_deepest_block = block_tree_->bestBlock();
  ASSERT_EQ(new_deepest_block.hash, hash);

  leaves = block_tree_->getLeaves();
  ASSERT_EQ(leaves.size(), 1);
  ASSERT_EQ(leaves[0], hash);

  children_res = block_tree_->getChildren(hash);
  ASSERT_TRUE(children_res);
  ASSERT_TRUE(children_res.value().empty());
}

/**
 * @given block tree with at least one block inside
 * @when adding a new block, which is not a child of any block inside
 * @then corresponding error is returned
 */
TEST_F(BlockTreeTest, AddBlockNoParent) {
  // GIVEN
  Block new_block{.header = makeBlockHeader(123, {})};

  // WHEN-THEN
  ASSERT_OUTCOME_ERROR(block_tree_->addBlock(new_block),
                       BlockTreeError::NO_PARENT);
}

/**
 * @given block tree with at least two blocks inside
 * @when finalizing a non-finalized block
 * @then finalization completes successfully
 */
TEST_F(BlockTreeTest, Finalize) {
  // GIVEN
  auto &&last_finalized_hash = block_tree_->getLastFinalized().hash;
  ASSERT_EQ(last_finalized_hash, kFinalizedBlockInfo.hash);

  Block new_block{.header = makeBlockHeader(kFinalizedBlockInfo.slot + 1,
                                            kFinalizedBlockInfo.hash)};
  auto hash = addBlock(new_block);

  Justification justification{{0x45, 0xF4}};
  auto encoded_justification = encode(justification).value();
  EXPECT_CALL(*storage_, getJustification(kFinalizedBlockInfo.hash))
      .WillRepeatedly(Return(outcome::success(justification)));
  EXPECT_CALL(*storage_, getJustification(hash))
      .WillRepeatedly(Return(outcome::failure(boost::system::error_code{})));
  EXPECT_CALL(*storage_, putJustification(justification, hash))
      .WillRepeatedly(Return(outcome::success()));
  EXPECT_CALL(*storage_, removeJustification(kFinalizedBlockInfo.hash))
      .WillRepeatedly(Return(outcome::success()));
  EXPECT_CALL(*storage_, getBlockHeader(hash))
      .WillRepeatedly(Return(outcome::success(new_block.header)));
  EXPECT_CALL(*storage_, getBlockBody(hash))
      .WillRepeatedly(Return(outcome::success(new_block.extrinsic)));
  EXPECT_CALL(*justification_storage_policy_,
              shouldStoreFor(finalized_block_header_, _))
      .WillOnce(Return(outcome::success(false)));

  // WHEN
  ASSERT_OUTCOME_SUCCESS(block_tree_->finalize(hash, justification));

  // THEN
  ASSERT_EQ(block_tree_->getLastFinalized().hash, hash);
}

/**
 * @given block tree with following topology (finalized blocks marked with an
 * asterisk):
 *
 *      +---B1---C1
 *     /
 * ---A*---B
 *
 * @when finalizing non-finalized block B1
 * @then finalization completes successfully: block B pruned, block C1
 persists,
 * metadata valid
 */
TEST_F(BlockTreeTest, FinalizeWithPruning) {
  // GIVEN
  auto &&A_finalized_hash = block_tree_->getLastFinalized().hash;
  ASSERT_EQ(A_finalized_hash, kFinalizedBlockInfo.hash);

  BlockHeader B_header =
      makeBlockHeader(kFinalizedBlockInfo.slot + 1, A_finalized_hash);
  BlockBody B_body{.preimages = {{.blob = {1}}}};
  Block B_block{B_header, B_body};
  auto B_hash = addBlock(B_block);

  BlockHeader B1_header =
      makeBlockHeader(kFinalizedBlockInfo.slot + 1, A_finalized_hash);
  BlockBody B1_body{.preimages = {{.blob = {2}}}};
  Block B1_block{B1_header, B1_body};
  auto B1_hash = addBlock(B1_block);

  BlockHeader C1_header =
      makeBlockHeader(kFinalizedBlockInfo.slot + 2, B1_hash);
  BlockBody C1_body{.preimages = {{.blob = {3}}}};
  Block C1_block{C1_header, C1_body};
  auto C1_hash = addBlock(C1_block);

  Justification justification{{0x45, 0xF4}};
  auto encoded_justification = encode(justification).value();
  EXPECT_CALL(*storage_, getJustification(B1_hash))
      .WillRepeatedly(Return(outcome::failure(boost::system::error_code{})));
  EXPECT_CALL(*storage_, putJustification(justification, B1_hash))
      .WillRepeatedly(Return(outcome::success()));
  EXPECT_CALL(*storage_, getBlockHeader(B1_hash))
      .WillRepeatedly(Return(outcome::success(B1_header)));
  EXPECT_CALL(*storage_, getBlockBody(B1_hash))
      .WillRepeatedly(Return(outcome::success(B1_body)));
  EXPECT_CALL(*storage_, getBlockBody(B_hash))
      .WillRepeatedly(Return(outcome::success(B1_body)));
  // EXPECT_CALL(*pool_, submitExtrinsic(_, _))
  //     .WillRepeatedly(
  //         Return(outcome::success(hasher_->blake2b_256(ByteVec{0xaa,
  //         0xbb}))));
  EXPECT_CALL(*storage_, removeJustification(kFinalizedBlockInfo.hash))
      .WillRepeatedly(Return(outcome::success()));
  EXPECT_CALL(*justification_storage_policy_,
              shouldStoreFor(finalized_block_header_, _))
      .WillOnce(Return(outcome::success(false)));

  // WHEN
  ASSERT_TRUE(block_tree_->finalize(B1_hash, justification));

  // THEN
  ASSERT_EQ(block_tree_->getLastFinalized().hash, B1_hash);
  ASSERT_EQ(block_tree_->getLeaves().size(), 1);
  ASSERT_EQ(block_tree_->bestBlock().hash, C1_hash);
}

/**
 * @given block tree with the following topology (finalized blocks marked with
 * an asterisk):
 *
 *      +---B1---C1
 *     /
 * ---A*---B
 *
 * @when finalizing non-finalized block B
 * @then finalization completes successfully: blocks B1, C1 pruned, metadata
 * valid
 */
TEST_F(BlockTreeTest, FinalizeWithPruningDeepestLeaf) {
  // GIVEN
  auto &&A_finalized_hash = block_tree_->getLastFinalized().hash;
  ASSERT_EQ(A_finalized_hash, kFinalizedBlockInfo.hash);

  BlockHeader B_header =
      makeBlockHeader(kFinalizedBlockInfo.slot + 1, A_finalized_hash);
  BlockBody B_body{.preimages = {{.blob = {1}}}};
  Block B_block{B_header, B_body};
  auto B_hash = addBlock(B_block);

  BlockHeader B1_header =
      makeBlockHeader(kFinalizedBlockInfo.slot + 1, A_finalized_hash);
  BlockBody B1_body{.preimages = {{.blob = {2}}}};
  Block B1_block{B1_header, B1_body};
  auto B1_hash = addBlock(B1_block);

  BlockHeader C1_header =
      makeBlockHeader(kFinalizedBlockInfo.slot + 2, B1_hash);
  BlockBody C1_body{.preimages = {{.blob = {3}}}};
  Block C1_block{C1_header, C1_body};
  auto C1_hash = addBlock(C1_block);

  Justification justification{{0x45, 0xF4}};
  auto encoded_justification = encode(justification).value();
  EXPECT_CALL(*storage_, putJustification(justification, B_hash))
      .WillRepeatedly(Return(outcome::success()));
  EXPECT_CALL(*storage_, getBlockHeader(B_hash))
      .WillRepeatedly(Return(outcome::success(B_header)));
  EXPECT_CALL(*storage_, getBlockBody(B_hash))
      .WillRepeatedly(Return(outcome::success(B_body)));
  EXPECT_CALL(*storage_, getBlockBody(B1_hash))
      .WillRepeatedly(Return(outcome::success(B1_body)));
  EXPECT_CALL(*storage_, getBlockBody(C1_hash))
      .WillRepeatedly(Return(outcome::success(C1_body)));
  // EXPECT_CALL(*pool_, submitExtrinsic(_, _))
  //     .WillRepeatedly(
  //         Return(outcome::success(hasher_->blake2b_256(ByteVec{0xaa,
  //         0xbb}))));
  EXPECT_CALL(*storage_, removeJustification(kFinalizedBlockInfo.hash))
      .WillRepeatedly(Return(outcome::success()));
  EXPECT_CALL(*justification_storage_policy_,
              shouldStoreFor(finalized_block_header_, _))
      .WillOnce(Return(outcome::success(false)));

  // WHEN
  ASSERT_TRUE(block_tree_->finalize(B_hash, justification));

  // THEN
  ASSERT_EQ(block_tree_->getLastFinalized().hash, B_hash);
  ASSERT_EQ(block_tree_->getLeaves().size(), 1);
  ASSERT_EQ(block_tree_->bestBlock().hash, B_hash);
}

std::shared_ptr<TreeNode> makeFullTree(size_t depth,
                                       const size_t branching_factor) {
  auto make_subtree = [&branching_factor](std::shared_ptr<TreeNode> parent,
                                          BlockNumber current_depth,
                                          BlockNumber max_depth,
                                          std::string name,
                                          auto &make_subtree) {
    BlockHash hash{};
    std::copy_n(name.begin(), name.size(), hash.begin());
    auto node = std::make_shared<TreeNode>(
        BlockInfo{current_depth, hash}, parent, false);
    if (current_depth + 1 == max_depth) {
      return node;
    }
    for (size_t i = 0; i < branching_factor; i++) {
      auto child = make_subtree(node,
                                current_depth + 1,
                                max_depth,
                                name + "_" + std::to_string(i),
                                make_subtree);
      node->children.push_back(child);
    }
    return node;
  };
  return make_subtree(
      std::shared_ptr<TreeNode>{nullptr}, 0, depth, "block0", make_subtree);
}

struct NodeProcessor {
  MOCK_METHOD(void, foo, (const TreeNode &), (const));
};

/**
 * @given block tree with at least three blocks inside
 * @when asking for a chain from the given block to top one
 * @then expected chain is returned
 */
TEST_F(BlockTreeTest, GetChainByBlockAscending) {
  // GIVEN
  BlockHeader header =
      makeBlockHeader(kFinalizedBlockInfo.slot + 1, kFinalizedBlockInfo.hash);
  BlockBody body{.preimages = {{.blob = {0}}}};
  Block new_block{header, body};
  auto hash1 = addBlock(new_block);

  header = makeBlockHeader(kFinalizedBlockInfo.slot + 2, hash1);
  body = BlockBody{.preimages = {{.blob = {0}}}};
  new_block = Block{header, body};
  auto hash2 = addBlock(new_block);

  std::vector<BlockHash> expected_chain{kFinalizedBlockInfo.hash, hash1, hash2};

  // WHEN
  ASSERT_OUTCOME_SUCCESS(
      chain, block_tree_->getBestChainFromBlock(kFinalizedBlockInfo.hash, 5));

  // THEN
  ASSERT_EQ(chain, expected_chain);
}

/**
 * @given block tree with at least three blocks inside
 * @when asking for chain from the given block to bottom
 * @then expected chain is returned
 */
TEST_F(BlockTreeTest, GetChainByBlockDescending) {
  // GIVEN
  BlockHeader header =
      makeBlockHeader(kFinalizedBlockInfo.slot + 1, kFinalizedBlockInfo.hash);
  BlockBody body{.preimages = {{.blob = {0}}}};
  Block new_block{header, body};
  auto hash1 = addBlock(new_block);

  header = makeBlockHeader(header.slot + 1, hash1);
  body = BlockBody{.preimages = {{.blob = {0}}}};
  new_block = Block{header, body};
  auto hash2 = addBlock(new_block);

  EXPECT_CALL(*storage_, getBlockHeader({kFinalizedBlockInfo.hash}))
      .WillOnce(Return(BlockTreeError::HEADER_NOT_FOUND));

  std::vector<BlockHash> expected_chain{hash2, hash1};

  // WHEN
  ASSERT_OUTCOME_SUCCESS(chain,
                         block_tree_->getDescendingChainToBlock(hash2, 5));

  // THEN
  ASSERT_EQ(chain, expected_chain);
}

/**
 * @given a block tree with one block in it
 * @when trying to obtain the best chain that contais a block, which is
 * present in the storage, but is not connected to the base block in the tree
 * @then BLOCK_NOT_FOUND error is returned
 */
TEST_F(BlockTreeTest, GetBestChain_DiscardedBlock) {
  BlockInfo target = kFirstBlockInfo;
  BlockInfo other(kFirstBlockInfo.slot, "OtherBlock#1"_arr32);
  EXPECT_CALL(*storage_, getBlockHash(target.slot))
      .WillRepeatedly(Return(std::vector{other.hash}));

  ASSERT_OUTCOME_ERROR(block_tree_->getBestContaining(target.hash),
                       BlockTreeError::BLOCK_ON_DEAD_END);
}

/**
 * @given a block tree with a chain with two blocks
 * @when trying to obtain the best chain with the second block
 * @then the second block hash is returned
 */
TEST_F(BlockTreeTest, GetBestChain_ShortChain) {
  auto target_hash = addHeaderToRepository(kFinalizedBlockInfo.hash, 1337);

  ASSERT_OUTCOME_SUCCESS(best_info,
                         block_tree_->getBestContaining(target_hash));
  ASSERT_EQ(best_info.hash, target_hash);
}

/**
 * @given a block tree with two branches-chains
 * @when trying to obtain the best chain containing the root of the split on two
 * chains
 * @then the longest chain with is returned
 */
TEST_F(BlockTreeTest, GetBestChain_TwoChains) {
  /*
          42   43  44  45  46   47

          LF - T - A - B - C1
                         \
                           C2 - D2
   */

  auto T_hash = addHeaderToRepository(kFinalizedBlockInfo.hash, 43);
  auto A_hash = addHeaderToRepository(T_hash, 44);
  auto B_hash = addHeaderToRepository(A_hash, 45);

  [[maybe_unused]]  //
  auto C1_hash = addHeaderToRepository(B_hash, 46);

  auto C2_hash = addHeaderToRepository(B_hash, 46);
  auto D2_hash = addHeaderToRepository(C2_hash, 47);

  ASSERT_OUTCOME_SUCCESS(best_info, block_tree_->getBestContaining(T_hash));
  ASSERT_EQ(best_info.hash, D2_hash);

  // test grandpa best chain selection when target block is not on best chain
  // https://github.com/paritytech/polkadot-sdk/pull/5153
  // https://github.com/paritytech/polkadot-sdk/blob/776e95748901b50ff2833a7d27ea83fd91fbf9d1/substrate/client/consensus/grandpa/src/tests.rs#L1823-L1931
  EXPECT_EQ(block_tree_->getBestContaining(C1_hash).value().hash, C1_hash);
}

/**
 * @given a block tree with two branches-chains
 * @when trying to obtain the best chain containing the root of the split on two
 * chains
 * @then the longest chain with is returned
 */
TEST_F(BlockTreeTest, Reorganize) {
  // GIVEN
  auto A_hash = addHeaderToRepository(kFinalizedBlockInfo.hash, 43);
  auto B_hash = addHeaderToRepository(A_hash, 44);

  //   42   43  44  45   46   47
  //
  //   LF - A - B

  // WHEN.1
  auto C1_hash = addHeaderToRepository(B_hash, 45, "1"_arr32);
  auto D1_hash = addHeaderToRepository(C1_hash, 46, "1"_arr32);
  auto E1_hash = addHeaderToRepository(D1_hash, 47, "1"_arr32);

  //   42   43  44  45   46   47
  //
  //   LF - A - B - C1 - D1 - E1

  // THEN.2
  ASSERT_TRUE(block_tree_->bestBlock() == BlockInfo(47, E1_hash));

  // WHEN.2
  auto C2_hash = addHeaderToRepository(B_hash, 45, "2"_arr32);
  auto D2_hash = addHeaderToRepository(C2_hash, 46, "2"_arr32);
  auto E2_hash = addHeaderToRepository(D2_hash, 47, "2"_arr32);

  //   42   43  44  45   46   47
  //
  //               _C2 - D2 - E2
  //              /
  //   LF - A - B - C1 - D1 - E1

  // THEN.2
  ASSERT_TRUE(block_tree_->bestBlock() == BlockInfo(47, E1_hash));

  // WHEN.3
  EXPECT_CALL(*storage_, putJustification(_, _))
      .WillOnce(Return(outcome::success()));

  EXPECT_CALL(*storage_, getBlockBody(_))
      .WillRepeatedly(Return(outcome::success(BlockBody{})));

  EXPECT_CALL(*storage_, removeJustification(kFinalizedBlockInfo.hash))
      .WillRepeatedly(Return(outcome::success()));
  EXPECT_CALL(*justification_storage_policy_,
              shouldStoreFor(finalized_block_header_, _))
      .WillOnce(Return(outcome::success(false)));

  ASSERT_OUTCOME_SUCCESS(block_tree_->finalize(C2_hash, {}));

  //   42   43  44  45   46   47
  //
  //   LF - A - B - C2 - D2 - E2

  // THEN.3
  ASSERT_TRUE(block_tree_->bestBlock() == BlockInfo(47, E2_hash));
}

TEST_F(BlockTreeTest, CleanupObsoleteJustificationOnFinalized) {
  auto b43 = addHeaderToRepository(kFinalizedBlockInfo.hash, 43);
  auto b55 = addHeaderToRepository(b43, 55);
  auto b56 = addHeaderToRepository(b55, 56);
  EXPECT_CALL(*storage_, getBlockBody(b56)).WillOnce(Return(BlockBody{}));

  Justification new_justification{"justification_56"_vec};

  // shouldn't keep old justification
  EXPECT_CALL(*justification_storage_policy_,
              shouldStoreFor(finalized_block_header_, _))
      .WillOnce(Return(false));
  // store new justification
  EXPECT_CALL(*storage_, putJustification(new_justification, b56))
      .WillOnce(Return(outcome::success()));
  // remove old justification
  EXPECT_CALL(*storage_, removeJustification(kFinalizedBlockInfo.hash))
      .WillOnce(Return(outcome::success()));
  ASSERT_OUTCOME_SUCCESS(block_tree_->finalize(b56, new_justification));
}

TEST_F(BlockTreeTest, KeepLastFinalizedJustificationIfItShouldBeStored) {
  auto b43 = addHeaderToRepository(kFinalizedBlockInfo.hash, 43);
  auto b55 = addHeaderToRepository(b43, 55);
  auto b56 = addHeaderToRepository(b55, 56);
  EXPECT_CALL(*storage_, getBlockBody(b56)).WillOnce(Return(BlockBody{}));

  Justification new_justification{"justification_56"_vec};

  // shouldn't keep old justification
  EXPECT_CALL(*justification_storage_policy_,
              shouldStoreFor(finalized_block_header_, _))
      .WillOnce(Return(true));
  // store new justification
  EXPECT_CALL(*storage_, putJustification(new_justification, b56))
      .WillOnce(Return(outcome::success()));
  ASSERT_OUTCOME_SUCCESS(block_tree_->finalize(b56, new_justification));
}

/**
 * @given a block tree with two branches-chains
 * @when trying to obtain the best chain containing the root of the split on two
 * chains
 * @then the longest chain with is returned
 */
TEST_F(BlockTreeTest, GetBestBlock) {
  auto T_hash = addHeaderToRepository(kFinalizedBlockInfo.hash, 43);
  auto A_hash = addHeaderToRepository(T_hash, 44);
  auto B_hash = addHeaderToRepository(A_hash, 45);

  [[maybe_unused]]  //
  auto C1_hash = addHeaderToRepository(B_hash, 46);

  auto C2_hash = addHeaderToRepository(B_hash, 46);
  auto D2_hash = addHeaderToRepository(C2_hash, 47);

  auto C3_hash = addHeaderToRepository(B_hash, 46);
  auto D3_hash = addHeaderToRepository(C3_hash, 47);
  auto E3_hash = addHeaderToRepository(D3_hash, 48);
  auto F3_hash = addHeaderToRepository(E3_hash, 49);

  //  42   43  44  45  46   47   48   49   50
  //
  //                  _C1
  //                 /
  //  LF - T - A - B - C2 - D2
  //                 \_
  //                   C3 - D3 - E3 - F3

  {
    ASSERT_OUTCOME_SUCCESS(best_info, block_tree_->getBestContaining(T_hash));
    ASSERT_EQ(best_info.hash, F3_hash);
  }

  // ---------------------------------------------------------------------------

  auto E2_hash = addHeaderToRepository(D2_hash, 48, true);

  //  42   43  44  45  46   47   48   49   50
  //
  //                  _C1
  //                 /
  //  LF - T - A - B - C2 - D2 - E2*
  //                 \_
  //                   C3 - D3 - E3 - F3

  {
    ASSERT_OUTCOME_SUCCESS(best_info, block_tree_->getBestContaining(T_hash));
    ASSERT_EQ(best_info.hash, E2_hash);
  }

  // ---------------------------------------------------------------------------

  auto G3_hash = addHeaderToRepository(F3_hash, 50, true);

  //  42   43  44  45  46   47   48   49   50
  //
  //                  _C1
  //                 /
  //  LF - T - A - B - C2 - D2 - E2*
  //                 \_
  //                   C3 - D3 - E3 - F3 - G3**

  {
    ASSERT_OUTCOME_SUCCESS(best_info, block_tree_->getBestContaining(T_hash));
    ASSERT_EQ(best_info.hash, G3_hash);
  }

  // ---------------------------------------------------------------------------

  ASSERT_OUTCOME_SUCCESS(block_tree_->markAsRevertedBlocks({E3_hash}));

  //  42   43  44  45  46   47   48   49   50
  //
  //                  _C1
  //                 /
  //  LF - T - A - B - C2 - D2 - E2*
  //                 \_
  //                   C3 - D3 - E3 - F3 - G3**

  {
    ASSERT_OUTCOME_SUCCESS(best_info, block_tree_->getBestContaining(T_hash));
    ASSERT_EQ(best_info.hash, E2_hash);
  }
}
