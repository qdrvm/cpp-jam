/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/unwrap.hpp>

#include <morum/common.hpp>
#include <morum/db.hpp>
#include <morum/merkle_tree.hpp>

namespace morum {

  /**
   * Paged vector to make less allocations for nodes and keep them pinned in
   * memory when the vector is resized to avoid reference invalidation.
   * Has nothing to do with NOMT.
   */
  class FlatPagedNodeStorage final : public NodeStorage {
   public:
    static constexpr size_t PageSizeBytes = 4096;
    static constexpr size_t MaxNodesInPage = 4096 / sizeof(TreeNode);
    static_assert(MaxNodesInPage == 32);

    qtils::OptionalRef<TreeNode> get(NodeId idx) override {
      QTILS_ASSERT_LESS(idx / MaxNodesInPage, data.size());
      QTILS_ASSERT_LESS(
          idx % MaxNodesInPage, metadata[idx / MaxNodesInPage].node_num);
      return data[idx / MaxNodesInPage]->nodes[idx % MaxNodesInPage].node;
    }

    qtils::OptionalRef<TreeNode> get(const Hash32 &hash) override {
      auto it = hash_to_id.find(hash);
      if (it != hash_to_id.end()) {
        return get(it->second);
      }
      return std::nullopt;
    }

    template <typename... Args>
    NodeId allocate(Args &&...args) {
      if (metadata.empty() || metadata.back().node_num == MaxNodesInPage) {
        data.emplace_back(std::make_unique<Page>());
        metadata.emplace_back(PageMetadata{.node_num = 0});
      }
      std::construct_at<Page::UninitNode>(
          &data.back()->nodes[metadata.back().node_num],
          Page::UninitNode{std::forward<Args>(args)...});
      metadata.back().node_num++;
      return ((data.size() - 1) * MaxNodesInPage)
          + (metadata.back().node_num - 1);
    }

    virtual NodeId store(const TreeNode &node) override {
      return allocate(node);
    }

    bool empty() const override {
      return metadata.empty();
    }

    void reserve_nodes(size_t nodes) override {
      metadata.reserve(nodes / MaxNodesInPage);
      data.reserve(nodes / MaxNodesInPage);
    }

   private:
    struct Page {
      union UninitNode {
        UninitNode() : empty{} {}

        template <typename... Args>
        UninitNode(Args &&...args) : node{std::forward<Args>(args)...} {}

        struct {
        } empty;
        TreeNode node;
      };
      // can actually store two leaves in place of one branch
      std::array<UninitNode, PageSizeBytes / sizeof(UninitNode)> nodes;
    };

    struct PageMetadata {
      uint16_t node_num;
    };

    std::vector<PageMetadata> metadata;
    std::vector<std::unique_ptr<Page>> data;
    std::unordered_map<Hash32, NodeId> hash_to_id;
  };

  class ArchiveNodeLoader final : public NodeLoader {
   public:
    explicit ArchiveNodeLoader(std::shared_ptr<KeyValueStorage> node_storage)
        : node_storage{std::move(node_storage)} {}

    std::expected<std::optional<TreeNode>, StorageError> load(
        qtils::BitSpan<>, const Hash32 &hash) const override {
      Hash32 hash_copy = hash;
      hash_copy[0] &= 0xFE;
      ByteArray<sizeof(Leaf)>
          node_data;  // in-memory branches contain non-serializable metadata,
                      // so Leaf's size is used
      QTILS_UNWRAP(auto res, node_storage->read_to(hash_copy, node_data));
      if (!res) {
        return std::nullopt;
      }
      QTILS_ASSERT_EQ(*res, sizeof(Leaf));
      return deserialize_node(node_data);
    }

   private:
    std::shared_ptr<KeyValueStorage> node_storage;
  };

  class ArchiveTrieDb {
   public:
    explicit ArchiveTrieDb(
        std::shared_ptr<ColumnFamilyStorage<ColumnFamilyId>> storage)
        : storage_{storage},
          node_storage_{storage_->get_column_family(ColumnFamilyId::TREE_NODE)},
          value_storage_{storage_->get_column_family(ColumnFamilyId::TREE_VALUE)} {
      QTILS_ASSERT(storage_ != nullptr);
      QTILS_ASSERT(node_storage_ != nullptr);
      QTILS_ASSERT(value_storage_ != nullptr);
    }

    std::expected<std::optional<std::unique_ptr<MerkleTree>>, StorageError>
    load_tree(const Hash32 &root_hash) const {
      auto root_bytes_res = node_storage_->read(root_hash);
      QTILS_ASSERT_HAS_VALUE(root_bytes_res);
      QTILS_ASSERT(root_bytes_res->has_value());
      auto root_bytes = **root_bytes_res;
      auto root = morum::deserialize_node(root_bytes);
      return std::make_unique<MerkleTree>(root,
          std::make_unique<FlatPagedNodeStorage>(),
          std::make_shared<ArchiveNodeLoader>(node_storage_));
    }

    std::unique_ptr<MerkleTree> empty_tree() const {
      return std::make_unique<MerkleTree>(
          std::make_unique<FlatPagedNodeStorage>(),
          std::make_shared<NoopNodeLoader>());
    }

    std::expected<Hash32, StorageError> get_root_and_store(
        const MerkleTree &tree) {
      auto batch = storage_->start_batch();

      auto hash = tree.calculate_hash([&](const morum::TreeNode &n,
                                          qtils::ByteSpan serialized,
                                          qtils::ByteSpan hash,
                                          qtils::BitSpan<>) {
        morum::Hash32 hash_copy;
        std::ranges::copy(hash, hash_copy.begin());
        hash_copy[0] &= 0xFE;
        [[maybe_unused]] auto res =
            batch->write(ColumnFamilyId::TREE_NODE, hash_copy, serialized);
        QTILS_ASSERT(res);
        if (n.is_leaf()) {
          // cached_nodes_.emplace(hash_copy, n);
        } else {
          auto &branch = n.as_branch();
          // original node may contain child node ids which are not persistent
          morum::Branch b{branch.get_left_hash(), branch.get_right_hash()};
        }
        if (n.is_leaf()) {
          auto h_or_v = n.as_leaf().hash_or_value();
          if (auto *hash = std::get_if<morum::HashRef>(&h_or_v); hash) {
            if (auto value_opt = tree.get_cached_value(hash->get());
                value_opt) {
              batch
                  ->write(ColumnFamilyId::TREE_VALUE,
                      hash->get(),
                      value_opt.value())
                  .value();
            }
          }
        }
      });
      hash[0] &= 0xFE;

      [[maybe_unused]] auto res = storage_->write_batch(std::move(batch));
      QTILS_ASSERT_HAS_VALUE(res);

      return hash;
    }

   private:
    std::shared_ptr<ColumnFamilyStorage<ColumnFamilyId>> storage_;
    std::shared_ptr<KeyValueStorage> node_storage_;
    std::shared_ptr<KeyValueStorage> value_storage_;
  };

}  // namespace morum
