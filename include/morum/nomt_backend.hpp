/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <ranges>

#include <morum/archive_backend.hpp>
#include <morum/common.hpp>
#include <morum/merkle_tree.hpp>
#include <morum/tree_node.hpp>
#include <qtils/byte_arr.hpp>

namespace morum {

  /**
   * This branch representation doesn't store any additional data contrary to
   * Branch.
   */
  struct RawBranch {
    // mind that the left hash must have the first bit set to 0
    Hash32 left;
    Hash32 right;

    const Hash32 &get_child(uint8_t bit) const {
      return bit ? right : left;
    }

    bool has_child(uint8_t bit) const {
      return get_child(bit) != ZeroHash32;
    }
  };

  /**
   * This node representation doesn't store any additional data contrary to
   * TreeNode.
   */
  union RawNode {
    std::monostate unitialized;
    Leaf leaf;
    RawBranch branch;

    bool is_branch() const {
      return (reinterpret_cast<const uint8_t *>(this)[0] & 1) == 0;
    }

    bool is_leaf() const {
      return (reinterpret_cast<const uint8_t *>(this)[0] & 1) == 1;
    }
  };

  static_assert(sizeof(RawNode) == 64);

  // size of a page in which nodes are stored on disk
  static constexpr size_t DISK_PAGE_SIZE = 4096;
  static constexpr size_t PAGE_NODES = DISK_PAGE_SIZE / sizeof(RawNode) - 2;
  // depth of a rootless subtree stored in a page
  static constexpr size_t PAGE_LEVELS =
      __builtin_ctz(DISK_PAGE_SIZE / sizeof(RawNode)) - 1;

  struct Page {
    // unused space
    qtils::ByteArr<sizeof(RawNode) * 2> metadata;
    // i.e. for DISK_PAGE_SIZE 4096: 32 + 16 + 8 + 4 + 2 = 62
    std::array<RawNode, DISK_PAGE_SIZE / sizeof(RawNode) - 2> nodes;

    // follows branch nodes and returns nullopt if at any point in path the
    // corresponding child is absent
    qtils::OptionalRef<RawNode> get_node(qtils::BitSpan<> path);

    // ignores branch nodes and just return where the corresponding node is
    // supposed to be placed
    RawNode &get_node_unchecked(qtils::BitSpan<> path);

    qtils::FixedByteSpanMut<DISK_PAGE_SIZE> as_bytes() {
      return qtils::FixedByteSpanMut<DISK_PAGE_SIZE>{
          reinterpret_cast<uint8_t *>(this), DISK_PAGE_SIZE};
    }
  };
  static_assert(std::is_standard_layout_v<Page>);
  static_assert(sizeof(Page) == DISK_PAGE_SIZE);

  inline TreeNode raw_node_to_node(const RawNode &raw_node) {
    if (raw_node.is_leaf()) {
      return raw_node.leaf;
    } else {
      return morum::Branch{raw_node.branch.left, raw_node.branch.right};
    }
  }

  class NearlyOptimalNodeStorage
      : public std::enable_shared_from_this<NearlyOptimalNodeStorage> {
   public:
    explicit NearlyOptimalNodeStorage(std::shared_ptr<KeyValueStorage> storage)
        : storage_{storage} {
      QTILS_ASSERT(storage != nullptr);
    }

    std::expected<std::optional<RawNode>, StorageError> load_root_node() const;
    std::expected<std::optional<Page>, StorageError> load_terminal_page(
        qtils::BitSpan<> path) const;
    std::expected<std::optional<Page>, StorageError> load_page_direct(
        qtils::BitSpan<> path) const;

    static size_t get_node_idx(qtils::BitSpan<> path) {
      size_t offset{};
      for (uint8_t bit : path) {
        offset |= bit;
        offset <<= 1;
      }
      offset >>= 1;
      return (1 << (path.size_bits() - 1)) + offset;
    }

    struct PageKeyHash {
      size_t operator()(const qtils::ByteArr<33> &page_key) const {
        size_t result;
        std::copy_n(page_key.begin(),
            sizeof(size_t),
            reinterpret_cast<uint8_t *>(&result));
        return result;
      }
    };

    struct WriteBatch {
      WriteBatch(std::shared_ptr<NearlyOptimalNodeStorage> storage)
          : page_storage{storage} {}

      std::expected<void, StorageError> set(
          qtils::BitSpan<> path, const RawNode &node);

      std::expected<std::optional<RawNode>, StorageError> load(
          [[maybe_unused]] const Hash32 &hash, qtils::BitSpan<> path);

      std::shared_ptr<NearlyOptimalNodeStorage> page_storage;
      std::unordered_map<qtils::ByteArr<33>, Page, PageKeyHash> page_cache;
      std::optional<RawNode> root_node;
    };

    std::unique_ptr<WriteBatch> start_writing() {
      return std::make_unique<WriteBatch>(shared_from_this());
    }

    std::expected<void, StorageError> submit_batch(
        std::unique_ptr<WriteBatch> batch,
        ColumnFamilyStorage<ColumnFamilyId>::Batch &db_batch);

   private:
    static qtils::FixedByteVec<sizeof(Hash32) + 1> get_page_key(
        qtils::BitSpan<> path);

    std::shared_ptr<KeyValueStorage> storage_;
  };

  class NomtDb {
   public:
    using Clock = std::chrono::steady_clock;

    struct Metrics {
      size_t pages_loaded{};
      size_t pages_stored{};
      size_t nodes_loaded{};
      size_t values_loaded{};
      size_t nodes_stored{};
      size_t values_stored{};

      Clock::duration page_load_duration{};
      Clock::duration page_store_duration{};
      Clock::duration page_batch_write_duration{};
      Clock::duration value_batch_write_duration{};
    };

    explicit NomtDb(
        std::shared_ptr<ColumnFamilyStorage<ColumnFamilyId>> storage)
        : storage_{storage},
          page_storage_{std::make_shared<NearlyOptimalNodeStorage>(
              storage->get_column_family(ColumnFamilyId::TREE_PAGE))} {
      QTILS_ASSERT(storage_ != nullptr);
    }

    void reset_metrics() {
      metrics_ = Metrics{};
    }

    const Metrics &get_metrics() const {
      return metrics_;
    }

    std::expected<
        std::optional<std::unique_ptr<NearlyOptimalNodeStorage::WriteBatch>>,
        StorageError>
    start_writing_tree() const {
      return std::make_unique<NearlyOptimalNodeStorage::WriteBatch>(
          page_storage_);
    }

    std::expected<std::optional<std::unique_ptr<MerkleTree>>, StorageError>
    load_tree(const Hash32 &root) const;

    std::unique_ptr<MerkleTree> empty_tree() const;

    std::expected<Hash32, StorageError> get_root_and_store(
        const MerkleTree &tree);

   private:
    std::shared_ptr<ColumnFamilyStorage<ColumnFamilyId>> storage_;
    std::shared_ptr<NearlyOptimalNodeStorage> page_storage_;

    mutable Metrics metrics_{};
  };

}  // namespace morum
