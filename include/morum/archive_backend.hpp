/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/macro/common.hpp>

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
        qtils::BitSpan<>, const Hash32 &hash) const override;

   private:
    std::shared_ptr<KeyValueStorage> node_storage;
  };

  class ArchiveTrieDb {
   public:
    explicit ArchiveTrieDb(
        std::shared_ptr<ColumnFamilyStorage<ColumnFamilyId>> storage);

    std::expected<std::optional<std::unique_ptr<MerkleTree>>, StorageError>
    load_tree(const Hash32 &root_hash) const;

    std::unique_ptr<MerkleTree> empty_tree() const;

    std::expected<Hash32, StorageError> get_root_and_store(
        const MerkleTree &tree);

   private:
    std::shared_ptr<ColumnFamilyStorage<ColumnFamilyId>> storage_;
    std::shared_ptr<KeyValueStorage> node_storage_;
    std::shared_ptr<KeyValueStorage> value_storage_;
  };

}  // namespace morum
