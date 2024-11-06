/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cassert>
#include <concepts>
#include <expected>
#include <generator>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

#include <qtils/bytes.hpp>

#include <morum/common.hpp>
#include <morum/storage_adapter.hpp>
#include <morum/tree_node.hpp>

namespace morum {
  // used to store trie keys in an std::map, for example
  struct TrieKeyOrder {
    bool operator()(const morum::Hash32 &a, const morum::Hash32 &b) const {
      for (size_t i = 0; i < 32; i++) {
        unsigned char a_bits = reverse_bits(a[i]);
        unsigned char b_bits = reverse_bits(b[i]);
        if (a_bits < b_bits) {
          return true;
        }
        if (a_bits > b_bits) {
          return false;
        }
      }
      return false;
    }
  };

  /**
   * An interface to load nodes from 
   */
  class NodeLoader {
   public:
    virtual std::expected<std::optional<TreeNode>, StorageError> load(
        qtils::BitSpan<> path, const Hash32 &hash) const = 0;
  };

  class NoopNodeLoader final : public NodeLoader {
   public:
    virtual std::expected<std::optional<TreeNode>, StorageError> load(
        qtils::BitSpan<>, const Hash32 &) const override {
      return std::nullopt;
    }
  };

  class NodeStorage {
   public:
    virtual ~NodeStorage() = default;

    virtual qtils::OptionalRef<TreeNode> get(NodeId id) = 0;
    virtual qtils::OptionalRef<TreeNode> get(const Hash32 &hash) = 0;

    virtual NodeId store(const TreeNode &) = 0;

    virtual bool empty() const = 0;

    virtual void reserve_nodes(size_t nodes) = 0;
  };

  class MerkleTree {
   private:
    qtils::OptionalRef<TreeNode> get_root();

    qtils::OptionalRef<const TreeNode> get_root() const;

    // value hash or embedded value
    using CachedValue = std::variant<Hash32, qtils::FixedByteVector<32>>;

   public:
    explicit MerkleTree(const TreeNode &root,
        std::unique_ptr<NodeStorage> node_storage,
        std::shared_ptr<NodeLoader> node_loader)
        : nodes_{std::move(node_storage)}, loader_{std::move(node_loader)} {
      QTILS_ASSERT(nodes_ != nullptr);
      QTILS_ASSERT(loader_ != nullptr);
      [[maybe_unused]] auto root_id = nodes_->store(root);
      QTILS_ASSERT(root_id == 0);
    }

    explicit MerkleTree(std::unique_ptr<NodeStorage> node_storage,
        std::shared_ptr<NodeLoader> node_loader)
        : nodes_{std::move(node_storage)}, loader_{std::move(node_loader)} {
      QTILS_ASSERT(nodes_ != nullptr);
      QTILS_ASSERT(loader_ != nullptr);
    }

    std::expected<void, StorageError> set(
        const Hash32 &key, ByteVector &&value);

    std::expected<std::optional<qtils::ByteSpan>, StorageError> get(
        const Hash32 &key) const;

    std::expected<bool, StorageError> exists(const Hash32 &key) const;

    bool empty() const {
      return nodes_->empty();
    }

    struct ValueWithPath {
      qtils::ByteSpan key;
      qtils::ByteSpan value;

      qtils::ByteArray<32> path_storage;
      qtils::BitSpan<> path;
    };

    static void empty_visitor(const TreeNode &, qtils::ByteSpan, qtils::ByteSpan, qtils::BitSpan<>) {
    }

    template <std::invocable<const TreeNode &, qtils::ByteSpan, qtils::ByteSpan, qtils::BitSpan<>>
                  Visitor = decltype(empty_visitor)>
    Hash32 calculate_hash(const Visitor &visitor = empty_visitor) const {
      if (empty()) {
        return ZeroHash32;
      }
      Hash32 hash;
      if (get_root()->is_leaf()) {
        auto serialized = serialize_leaf(get_root()->as_leaf().get_key(),
            get_root()->as_leaf().hash_or_value());
        hash = blake2b_256(serialized);
        visitor(*get_root(), serialized, hash, qtils::BitSpan<>{});
      } else {
        ByteArray<sizeof(Hash32)> path_storage;
        hash = calculate_hash(
            *get_root(), visitor, qtils::BitSpan<uint8_t>{path_storage, 0, 0});
      }
      return hash;
    }

    using ValueHash = Hash32;

    // if the value is cached, it means it's 'dirty' and should be written to
    // the DB
    std::optional<qtils::ByteSpan> get_cached_value(const ValueHash &hash) const {
      auto it = value_cache_.find(hash);
      if (it != value_cache_.end()) {
        return it->second;
      }
      return std::nullopt;
    }

   private:
    std::expected<std::optional<size_t>, StorageError> get_child_idx(
        Branch &branch, int8_t bit, qtils::BitSpan<> path) const;

    std::expected<qtils::ByteSpan, StorageError> get_value(
        Leaf::HashOrValue hash_or_value) const;

    struct NodeWithPath {
      size_t node_idx;
      qtils::BitSpan<> path;
    };
    // find the lowest existing branch on the path described by key
    std::expected<std::optional<NodeWithPath>, StorageError> find_parent_branch(
        const Hash32 &key);

    std::expected<std::optional<NodeId>, StorageError> find_leaf(
        const Hash32 &key) const;

    size_t create_leaf_node(const Hash32 &key, qtils::Bytes &&value);

    void replace_leaf_with_branch(
        size_t path_len, size_t old_leaf_idx, size_t new_leaf_idx);

    template <
        std::invocable<const TreeNode &, qtils::ByteSpan, qtils::ByteSpan, qtils::BitSpan<>> Visitor>
    Hash32 calculate_hash(const TreeNode &n,
        const Visitor &visitor,
        qtils::BitSpan<uint8_t> path) const {
      Hash32 hash;
      if (n.is_branch()) {
        auto calculate_child_hash = [this, visitor, path](
                                        Branch &n, uint8_t bit) mutable {
          Hash32 hash;
          if (auto hash_opt = n.get_child_hash(bit); hash_opt.has_value()) {
            hash = *hash_opt;
          } else if (auto idx = n.get_child_idx(bit); idx.has_value()) {
            path.end_bit++;
            path.set_bit(path.end_bit - 1, bit);
            hash = calculate_hash(*nodes_->get(*idx), visitor, path);
            path.end_bit--;
            n.set_child(bit, hash);
          } else {
            hash = ZeroHash32;
            n.set_child(bit, hash);
          }
          return hash;
        };
        // branch won't be mutated per se, only calculated hashes would be
        // cached in it
        auto &branch = const_cast<Branch &>(n.as_branch());
        Hash32 left_t = calculate_child_hash(branch, 0);
        Hash32 right_t = calculate_child_hash(branch, 1);
        auto serialized = serialize_branch(left_t, right_t);
        hash = blake2b_256(serialized);
        visitor(n, serialized, hash, qtils::BitSpan<>{path});
      } else {
        auto serialized =
            serialize_leaf(n.as_leaf().get_key(), n.as_leaf().hash_or_value());
        hash = blake2b_256(serialized);
        visitor(n, serialized, hash, qtils::BitSpan<>{path});
      }
      return hash;
    }

    // mutable because we may need to fetch a node from disk to memory in
    // const methods
    mutable std::shared_ptr<NodeStorage> nodes_;
    mutable std::shared_ptr<NodeLoader> loader_;
    mutable std::shared_ptr<StorageAdapter> value_storage_;
    mutable std::unordered_map<ValueHash, qtils::Bytes> value_cache_;
  };

}  // namespace morum
