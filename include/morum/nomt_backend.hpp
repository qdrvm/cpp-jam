/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <morum/archive_backend.hpp>
#include <morum/common.hpp>
#include <morum/merkle_tree.hpp>
#include <morum/tree_node.hpp>

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
    uint8_t metadata[sizeof(RawNode) * 2];
    // i.e. for DISK_PAGE_SIZE 4096: 32 + 16 + 8 + 4 + 2 = 62
    std::array<RawNode, DISK_PAGE_SIZE / sizeof(RawNode) - 2> nodes;

    // follows branch nodes and returns nullopt if at any point in path the
    // corresponding child is absent
    qtils::OptionalRef<RawNode> get_node(qtils::BitSpan<> path) {
      QTILS_ASSERT_GREATER(path.size_bits(), 0);
      QTILS_ASSERT(path.size_bits() <= PAGE_LEVELS);
      RawNode *current = &nodes[path[0]];
      size_t offset{path[0]};
      for (auto [level, bit] : path.skip_first(1) | std::views::enumerate) {
        if (current->is_leaf() || !current->branch.has_child(bit)) {
          return std::nullopt;
        }
        offset = offset | (bit << (level + 1));
        size_t level_offset = (1 << (level + 2)) - 2;
        current = &nodes[level_offset + offset];
      }
      return *current;
    }

    // ignores branch nodes and just return where the corresponding node is
    // supposed to be placed
    RawNode &get_node_unchecked(qtils::BitSpan<> path) {
      QTILS_ASSERT_GREATER(path.size_bits(), 0);
      QTILS_ASSERT(path.size_bits() <= PAGE_LEVELS);
      size_t offset = path.get_as_byte(0, path.size_bits());
      size_t level_offset = (1 << path.size_bits()) - 2;
      return nodes[level_offset + offset];
    }

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

    std::expected<std::optional<RawNode>, StorageError> load_root_node() const {
      RawNode root{};
      QTILS_UNWRAP(auto exists,
          storage_->read_to({},
              qtils::ByteSpanMut{
                  reinterpret_cast<uint8_t *>(&root), sizeof(RawNode)}));
      if (exists) {
        return root;
      }
      return std::nullopt;
    }

    std::expected<std::optional<Page>, StorageError> load_terminal_page(
        qtils::BitSpan<> path) const {
      std::optional<Page> previous_page;
      for (size_t depth = 0; depth <= path.size_bits() / PAGE_LEVELS; depth++) {
        QTILS_UNWRAP(auto opt_page,
            load_page_direct(path.subspan(0, depth * PAGE_LEVELS)));
        if (!opt_page) {
          if (previous_page) {
            MORUM_TRACE(
                "load terminal page, at path {}, found at depth {} pages/{} "
                "bit",
                path,
                depth,
                depth * PAGE_LEVELS);
          } else {
            MORUM_TRACE("load terminal page, at path {}, not found",
                path,
                depth,
                depth * PAGE_LEVELS);
          }

          return previous_page;
        }
        auto &page = *opt_page;
        previous_page = page;
        auto node =
            page.get_node(path.subspan(depth * PAGE_LEVELS, PAGE_LEVELS));
        if (!node || node->is_leaf()) {
          MORUM_TRACE(
              "load terminal page, at path {}, found at depth {} pages/{} bit",
              path,
              depth,
              depth * PAGE_LEVELS);

          return page;
        }
      }
      QTILS_ASSERT(
          !"unreachable: all paths must end with a leaf node or a null child of a branch node");
      std::unreachable();
    }

    std::expected<std::optional<Page>, StorageError> load_page_direct(
        qtils::BitSpan<> path) const {
      if (path.size_bits() == 0) {
        return std::nullopt;
      }
      std::expected<std::optional<Page>, StorageError> page{};

      auto key = get_page_key(path);
      QTILS_UNWRAP(auto res,
          storage_->read_to(key.span(),
              qtils::ByteSpanMut{
                  reinterpret_cast<uint8_t *>(&**page), sizeof(Page)}));
      if (!res) {
        MORUM_TRACE("load page directly, at path {}, key {}, none found",
            path,
            key.span());
        return std::nullopt;
      }
      QTILS_ASSERT(res.value() == sizeof(Page));
      MORUM_TRACE("load page directly, at path {}, key {}, found successfully",
          path,
          key.span());
      return page;
    }

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
      size_t operator()(const ByteArray<33> &page_key) const {
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
          qtils::BitSpan<> path, const RawNode &node) {
        if (path.size_bits() == 0) {
          root_node = node;
          return {};
        }
        qtils::OptionalRef<Page> page{};
        Page page_data{};
        auto [key, _] = get_page_key(path);
        if (auto it = page_cache.find(key); it != page_cache.end()) {
          page = it->second;
        } else {
          QTILS_UNWRAP(auto page_opt, page_storage->load_page_direct(path));
          if (page_opt) {
            page_data = *page_opt;
          }
          page = page_data;
          page_cache.emplace(key, page_data);
        }
        auto offset = PAGE_LEVELS * ((path.size_bits() - 1) / PAGE_LEVELS);
        page->get_node_unchecked(
            path.subspan(offset, path.size_bits() - offset)) = node;
        return {};
      }

      std::expected<std::optional<RawNode>, StorageError> load(
          [[maybe_unused]] const Hash32 &hash, qtils::BitSpan<> path) {
        if (path.size_bits() == 0) {
          QTILS_UNWRAP(auto root_opt, page_storage->load_root_node());
          if (!root_opt) {
            MORUM_TRACE("load root node, not found");
            return std::nullopt;
          }
          MORUM_TRACE("load root node, success");
          return *root_opt;
        }
        QTILS_UNWRAP(auto page_opt, page_storage->load_terminal_page(path));
        if (page_opt) {
          auto raw_node = page_opt->get_node(path);
          if (!raw_node) {
            MORUM_TRACE(
                "load node hash {} path {}, not found in page", hash, path);
            return std::nullopt;
          }
          MORUM_TRACE("load node hash {} path {}, success", hash, path);
          return *raw_node;
        }
        MORUM_TRACE("load node hash {} path {}, page not found", hash, path);
        return std::nullopt;
      }

      std::shared_ptr<NearlyOptimalNodeStorage> page_storage;
      std::unordered_map<ByteArray<33>, Page, PageKeyHash> page_cache;
      std::optional<RawNode> root_node;
    };

    std::unique_ptr<WriteBatch> start_writing() {
      return std::make_unique<WriteBatch>(shared_from_this());
    }

    std::expected<void, StorageError> submit_batch(
        std::unique_ptr<WriteBatch> batch,
        ColumnFamilyStorage<ColumnFamilyId>::Batch &db_batch) {
      QTILS_ASSERT(batch != nullptr);
      if (batch->root_node) {
        if (batch->root_node->is_branch()) {
          QTILS_UNWRAP_void(db_batch.write(ColumnFamilyId::TREE_PAGE,
              qtils::ByteSpan{},
              serialize_branch(batch->root_node->branch.left,
                  batch->root_node->branch.right)));
        } else {
          QTILS_UNWRAP_void(db_batch.write(ColumnFamilyId::TREE_PAGE,
              qtils::ByteSpan{},
              serialize_leaf(batch->root_node->leaf.get_key(),
                  batch->root_node->leaf.hash_or_value())));
        }
      }

      for (auto &[key, page] : batch->page_cache) {
        QTILS_UNWRAP_void(db_batch.write(ColumnFamilyId::TREE_PAGE,
            qtils::ByteSpan{key.data(), key[0]},
            page.as_bytes()));
      }
      return {};
    }

   private:
    static qtils::FixedByteVector<sizeof(Hash32) + 1> get_page_key(
        qtils::BitSpan<> path) {
      path = path.subspan(0, (path.size_bits() / PAGE_LEVELS) * PAGE_LEVELS);
      qtils::FixedByteVector<sizeof(Hash32) + 1> key_storage{};
      key_storage.data[0] = path.size_bits();
      std::copy(path.begin(), path.end(), key_storage.data.begin() + 1);
      key_storage.size = path.size_bits() / 8 + 1;
      return key_storage;
    }

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
      QTILS_ASSERT(page_storage != nullptr);
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
    load_tree(const Hash32 &root) const {
      auto storage = std::make_unique<FlatPagedNodeStorage>();
      class NomtNodeLoader final : public NodeLoader {
       public:
        NomtNodeLoader(const NomtDb &db, FlatPagedNodeStorage &storage)
            : db{db}, storage{storage} {}

        virtual std::expected<std::optional<TreeNode>, StorageError> load(
            qtils::BitSpan<> path, const Hash32 &hash_copy) const override {
          Hash32 hash = hash_copy;
          hash[0] &= 0xFE;
          if (path.size_bits() == 0) {
            QTILS_UNWRAP(auto root_opt, db.page_storage_->load_root_node());
            if (!root_opt) {
              MORUM_TRACE("load root node, not found");
              return std::nullopt;
            }
            MORUM_TRACE("load root node, success");
            db.metrics_.nodes_loaded++;

            return raw_node_to_node(*root_opt);
          }
          QTILS_UNWRAP(
              auto page_opt, db.page_storage_->load_terminal_page(path));
          if (page_opt) {
            auto raw_node = page_opt->get_node(path);
            if (!raw_node) {
              MORUM_TRACE(
                  "load node hash {} path {}, not found in page", hash, path);
              return std::nullopt;
            }
            MORUM_TRACE("load node hash {} path {}, success", hash, path);
            db.metrics_.nodes_loaded++;
            return raw_node_to_node(*raw_node);
          }
          MORUM_TRACE("load node hash {} path {}, page not found", hash, path);
          return std::nullopt;
        }

        const NomtDb &db;
        FlatPagedNodeStorage &storage;
      };
      auto loader = std::make_shared<NomtNodeLoader>(*this, *storage);
      QTILS_UNWRAP(auto root_node_opt, loader->load(qtils::BitSpan<>{}, root));
      if (!root_node_opt) {
        return std::nullopt;
      }
      return std::make_unique<MerkleTree>(
          std::move(storage), std::make_shared<NoopNodeLoader>());
    }

    std::unique_ptr<MerkleTree> empty_tree() const {
      auto storage = std::make_unique<FlatPagedNodeStorage>();

      return std::make_unique<MerkleTree>(
          std::move(storage), std::make_shared<NoopNodeLoader>());
    }

    std::expected<Hash32, StorageError> get_root_and_store(
        const MerkleTree &tree) {
      auto page_batch = page_storage_->start_writing();
      auto total_batch = storage_->start_batch();

      auto hash = tree.calculate_hash([&](const TreeNode &n,
                                          qtils::ByteSpan,
                                          qtils::ByteSpan hash,
                                          qtils::BitSpan<> path) {
        morum::Hash32 hash_copy;
        std::ranges::copy(hash, hash_copy.begin());
        hash_copy[0] &= 0xFE;
        RawNode raw_node{};
        if (n.is_leaf()) {
          raw_node.leaf = n.as_leaf();
        } else {
          raw_node.branch.left = n.as_branch().get_left_hash_raw();
          raw_node.branch.right = n.as_branch().get_right_hash_raw();
          QTILS_ASSERT(raw_node.branch.left != Branch::NoHash);
          QTILS_ASSERT(raw_node.branch.right != Branch::NoHash);
        }
        MORUM_TRACE("store node hash {} path {}", hash, path);
        [[maybe_unused]] auto res = page_batch->set(path, raw_node);
        QTILS_ASSERT(res);

        if (n.is_leaf()) {
          auto h_or_v = n.as_leaf().hash_or_value();
          if (auto *hash = std::get_if<morum::HashRef>(&h_or_v); hash) {
            if (auto value_opt = tree.get_cached_value(hash->get());
                value_opt) {
              total_batch->write(ColumnFamilyId::TREE_VALUE, hash->get(), value_opt.value()).value();
            }
          }
        }
      });
      hash[0] &= 0xFE;

      auto page_batch_start = Clock::now();
      [[maybe_unused]] auto res =
          page_storage_->submit_batch(std::move(page_batch), *total_batch);
      QTILS_ASSERT_HAS_VALUE(res);
      metrics_.page_batch_write_duration += Clock::now() - page_batch_start;

      auto value_batch_start = Clock::now();
      [[maybe_unused]] auto res2 =
          storage_->write_batch(std::move(total_batch));
      QTILS_ASSERT_HAS_VALUE(res2);
      metrics_.value_batch_write_duration += Clock::now() - value_batch_start;

      return hash;
    }

   private:
    std::shared_ptr<ColumnFamilyStorage<ColumnFamilyId>> storage_;
    std::shared_ptr<NearlyOptimalNodeStorage> page_storage_;

    mutable Metrics metrics_{};
  };

}  // namespace morum
