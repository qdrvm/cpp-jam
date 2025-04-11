/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>

#include <morum/merkle_tree.hpp>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <blake2.h>
#include <fmt/ranges.h>
#include <qtils/byte_utils.hpp>
#include <qtils/hex.hpp>
#include <qtils/macro/unwrap.hpp>

#include <morum/common.hpp>
#include <morum/db.hpp>
#include <morum/tree_node.hpp>

namespace morum {

  template <std::integral T>
  constexpr T div_round_up(T a, T b) {
    return (a + b - 1) / b;
  }

  Hash32 blake2b_256(qtils::ByteSpan bytes) {
    Hash32 res;
    blake2(res.data(), res.size(), bytes.data(), bytes.size(), nullptr, 0);
    return res;
  }

  qtils::ByteArray<64> serialize_leaf(
      const qtils::ByteArray<31> &key, const Hash32 &value_hash) {
    qtils::ByteArray<64> bytes{};
    bytes[0] = 0x3;
    std::copy_n(key.begin(), 31, bytes.begin() + 1);
    std::copy_n(value_hash.begin(), value_hash.size(), bytes.begin() + 32);
    return bytes;
  }

  qtils::ByteArray<64> serialize_leaf(
      const qtils::ByteArray<31> &key, qtils::ByteSpan value) {
    qtils::ByteArray<64> bytes{};
    Hash32 value_hash;
    if (value.size() <= 32) {
      bytes[0] = 0x01 | (value.size() << 2);
      std::copy_n(value.begin(), value.size(), value_hash.begin());
      std::fill_n(value_hash.begin() + value.size(), 32 - value.size(), 0);
    } else {
      bytes[0] = 0x3;
      value_hash = blake2b_256(value);
    }
    std::copy_n(key.begin(), 31, bytes.begin() + 1);
    std::copy_n(value_hash.begin(), value_hash.size(), bytes.begin() + 32);
    return bytes;
  }

  qtils::ByteArray<64> serialize_leaf(
      const qtils::ByteArray<31> &key, const Leaf::HashOrValue &value) {
    if (std::holds_alternative<HashRef>(value)) {
      return serialize_leaf(key, std::get<HashRef>(value).get());
    }
    return serialize_leaf(key, std::get<qtils::ByteSpan>(value));
  }

  qtils::ByteArray<64> serialize_branch(
      const Hash32 &left, const Hash32 &right) {
    qtils::ByteArray<64> bytes{};
    bytes[0] = 0xFE & left[0];
    std::copy_n(left.begin() + 1, 31, bytes.begin() + 1);
    std::copy_n(right.begin(), 32, bytes.begin() + 32);
    return bytes;
  }

  TreeNode deserialize_node(qtils::ByteSpan bytes) {
    qtils::BitSpan<> bits{bytes};
    if (bits[0] == 0) {
      Branch branch;
      if (!std::ranges::equal(bytes.subspan(0, 32), ZeroHash32)) {
        branch.set_left(bytes.subspan<0, 32>());
      }
      if (!std::ranges::equal(bytes.subspan(32, 32), ZeroHash32)) {
        branch.set_right(bytes.subspan<32, 32>());
      }
      return branch;
    }
    qtils::ByteArray<31> key;
    std::ranges::copy(bytes.subspan(1, 31), key.begin());
    if (bits[1] == 0) {
      size_t value_size = bytes[0] >> 3;
      return Leaf{Leaf::EmbeddedTag{}, key, bytes.subspan(32, value_size)};
    }
    return Leaf{Leaf::HashedTag{}, key, bytes.subspan<32, 32>()};
  }

  qtils::OptionalRef<TreeNode> MerkleTree::get_root() {
    if (nodes_->empty()) {
      return std::nullopt;
    }
    return nodes_->get(0);
  }

  qtils::OptionalRef<const TreeNode> MerkleTree::get_root() const {
    if (nodes_->empty()) {
      return std::nullopt;
    }
    return nodes_->get(0);
  }

  std::expected<void, StorageError> MerkleTree::set(
      const Hash32 &key, ByteVector &&value) {
    MORUM_TRACE("Set {}", BitSpan<>{key});
    if (empty()) {
      MORUM_TRACE("New root");
      [[maybe_unused]] auto id = create_leaf_node(key, std::move(value));
      QTILS_ASSERT(id == 0);
      return {};
    }
    QTILS_UNWRAP(const auto &parent_opt, find_parent_branch(key));
    if (!parent_opt) {
      // parent must be a leaf root
      QTILS_ASSERT(get_root()->is_leaf());
      MORUM_TRACE("New root branch");

      size_t new_leaf = create_leaf_node(key, std::move(value));
      replace_leaf_with_branch(0, 0, new_leaf);

      return {};
    }
    auto &[parent_idx, path] = *parent_opt;
    MORUM_TRACE("Parent path {}", path);

    qtils::BitSpan<> path_left{key, path.end_bit, key.size() * 8};
    auto new_leaf_idx = create_leaf_node(key, std::move(value));
    QTILS_UNWRAP(auto old_leaf_opt,
        get_child_idx(nodes_->get(parent_idx)->as_branch(),
            path[path.end_bit - 1],
            path));
    if (old_leaf_opt) {
      MORUM_TRACE("Replace leaf at bit {}, idx {} with idx {}",
          path[path.end_bit - 1],
          *old_leaf_opt,
          new_leaf_idx);
      replace_leaf_with_branch(
          path_left.start_bit /*+ 1*/, *old_leaf_opt, new_leaf_idx);
    } else {
      MORUM_TRACE(
          "New leaf at bit {}, idx {}", path[path.end_bit - 1], new_leaf_idx);
      nodes_->get(parent_idx)
          ->as_branch()
          .set_child(path[path.end_bit - 1], new_leaf_idx);
    }

    NodeId current_id = 0;
    TreeNode *current = &*nodes_->get(current_id);
    size_t i = 0;
    while (current->is_branch()) {
      auto bit = get_bit(key, i);
      current->as_branch().reset_child_hash(bit);
      ++i;
      auto id = current->as_branch().get_child_idx(bit).value();
      current_id = id;
      current = &nodes_->get(id).value();
    }
    return {};
  }

  std::expected<std::optional<qtils::ByteSpan>, StorageError> MerkleTree::get(
      const Hash32 &key) const {
    QTILS_UNWRAP(const auto &leaf_id_opt, find_leaf(key));
    if (!leaf_id_opt) {
      return std::nullopt;
    }
    auto leaf_id = *leaf_id_opt;
    auto &leaf = nodes_->get(leaf_id)->as_leaf();
    QTILS_UNWRAP(const auto &value, get_value(leaf.hash_or_value()));

    return value;
  }

  std::expected<bool, StorageError> MerkleTree::exists(
      const Hash32 &key) const {
    QTILS_UNWRAP(const auto &leaf_opt, find_leaf(key));
    return leaf_opt.has_value();
  }

  std::expected<std::optional<size_t>, StorageError> MerkleTree::get_child_idx(
      Branch &branch, int8_t bit, qtils::BitSpan<> path) const {
    QTILS_ASSERT(bit == 0 || bit == 1);
    if (auto idx = branch.get_child_idx(bit); idx.has_value()) {
      return *idx;
    }
    if (auto hash = branch.get_child_hash(bit); hash.has_value()) {
      QTILS_UNWRAP(auto node_opt, loader_->load(path, *hash));
      if (!node_opt) {
        return std::unexpected(StorageError{fmt::format(
            "Node hash {} path {} not found", qtils::Hex{*hash}, path)});
      }
      auto node_id = nodes_->store(*node_opt);
      branch.set_child(bit, node_id);
      return node_id;
    }

    return std::nullopt;
  }

  std::expected<qtils::ByteSpan, StorageError> MerkleTree::get_value(
      Leaf::HashOrValue hash_or_value) const {
    if (auto *embedded_val = std::get_if<qtils::ByteSpan>(&hash_or_value);
        embedded_val) {
      return qtils::ByteSpan{embedded_val->data(), embedded_val->size()};
    }
    QTILS_ASSERT(std::holds_alternative<HashRef>(hash_or_value));
    auto &hash = std::get<HashRef>(hash_or_value).get();
    if (auto it = value_cache_.find(hash); it != value_cache_.end()) {
      return it->second;
    }
    QTILS_UNWRAP(const auto & value_opt, value_storage_->read(hash));
    if (!value_opt) {
      return std::unexpected(StorageError{"Value missing"});
    }
    auto [res, success] = value_cache_.emplace(hash, std::move(*value_opt));
    QTILS_ASSERT(success);
    return res->second;
  }

  std::expected<std::optional<MerkleTree::NodeWithPath>, StorageError>
  MerkleTree::find_parent_branch(const Hash32 &key) {
    qtils::BitSpan<> path{key, 0, 0};
    if (empty()) {
      return std::nullopt;
    }
    // SAFETY: the check that tree is not empty is above
    auto &root = *get_root();
    if (root.is_leaf()) {
      return std::nullopt;
    }
    TreeNode *current = &root;
    size_t current_idx{};
    while (current->is_branch()) {
      auto bit = get_bit(key, path.size_bits());
      path.end_bit++;
      QTILS_UNWRAP(const auto & child_opt,
          get_child_idx(const_cast<Branch &>(current->as_branch()), bit, path));
      if (!child_opt || nodes_->get(*child_opt)->is_leaf()) {
        MORUM_TRACE("parent is {}, path {}", current_idx, path);
        return NodeWithPath{current_idx, path};
      }
      current = &*nodes_->get(*child_opt);
      current_idx = *child_opt;
    }
    std::unreachable();
  }

  std::expected<std::optional<NodeId>, StorageError> MerkleTree::find_leaf(
      const Hash32 &key) const {
    if (empty()) {
      return std::nullopt;
    }
    // SAFETY: check that tree is not empty above
    auto &root = *get_root();
    if (root.is_leaf()) {
      // TODO: what if the missing byte mismatches?
      if (std::ranges::equal(
              root.as_leaf().get_key(), qtils::ByteSpan{key}.subspan(0, 31))) {
        return 0;
      }
      return std::nullopt;
    }
    const TreeNode *current = &root;
    NodeId current_id = 0;
    size_t path_len = 0;
    while (current->is_branch()) {
      QTILS_UNWRAP(const auto & child_opt,
          get_child_idx(const_cast<Branch &>(current->as_branch()),
              get_bit(key, path_len),
              qtils::BitSpan<>{key, 0, path_len}));
      path_len++;
      if (!child_opt) {
        return std::nullopt;
      }
      current_id = *child_opt;
      current = &*nodes_->get(*child_opt);
    }
    if (!current->is_leaf()) {
      return std::nullopt;
    }
    auto &leaf = current->as_leaf();

    // TODO: what if the dropped byte is different?
    if (std::ranges::equal(
            leaf.get_key(), qtils::ByteSpan{key}.subspan(0, 31))) {
      return current_id;
    }
    return std::nullopt;
  }

  size_t MerkleTree::create_leaf_node(const Hash32 &key, qtils::Bytes &&value) {
    ByteArray<31> key_part =
        qtils::array_from_span<31>(qtils::ByteSpan{key}.subspan(0, 31));
    if (value.size() > 32) {
      Hash32 hash = blake2b_256(value);
      value_cache_[hash] = std::move(value);
      MORUM_TRACE("insert value with hash {} to value cache", hash);
      return nodes_->store(Leaf{Leaf::HashedTag{}, key_part, hash});
    }
    return nodes_->store(Leaf{Leaf::EmbeddedTag{},
        key_part,
        qtils::ByteSpan{value.data(), value.size()}});
  }

  void MerkleTree::replace_leaf_with_branch(
      size_t path_len, size_t old_leaf_idx, size_t new_leaf_idx) {
    Leaf &old_leaf = nodes_->get(old_leaf_idx)->as_leaf();
    Leaf &new_leaf = nodes_->get(new_leaf_idx)->as_leaf();

    ByteArray<31> old_key = old_leaf.get_key();
    qtils::BitSpan<> old_key_bits{old_key};
    qtils::BitSpan<> new_key_bits{new_leaf.get_key()};
    size_t common_key_len = std::distance(old_key_bits.begin(),
        std::ranges::mismatch(old_key_bits, new_key_bits).in1);
    MORUM_TRACE(
        "replace leaf #{} with leaf #{}, path len {}, common key len {}",
        old_leaf_idx,
        new_leaf_idx,
        path_len,
        common_key_len);
    // size of the key part in a node
    if (common_key_len == sizeof(old_key)) {
      // leaf keys are identical
      old_leaf = new_leaf;
      return;
    }
    QTILS_ASSERT_GREATER_EQ(common_key_len, path_len);

    size_t new_old_leaf_idx = nodes_->store(old_leaf);
    MORUM_TRACE("move old leaf to #{}", new_old_leaf_idx);

    auto [left_idx, right_idx] = old_key_bits[common_key_len] == 0
        ? std::make_pair(new_old_leaf_idx, new_leaf_idx)
        : std::make_pair(new_leaf_idx, new_old_leaf_idx);

    MORUM_TRACE("new left leaf is #{}, right leaf is #{}", left_idx, right_idx);

    QTILS_ASSERT_LESS(path_len + 1, old_key_bits.size_bits());

    // add intermediate branches
    *nodes_->get(old_leaf_idx) = TreeNode{Branch{}};
    MORUM_TRACE("new branch #{} in place of old leaf", old_leaf_idx);

    NodeId current_branch = old_leaf_idx;
    for (size_t i = path_len; i < common_key_len; i++) {
      auto new_node_idx = nodes_->store(Branch{});
      nodes_->get(current_branch)
          ->as_branch()
          .set_child(old_key_bits[i], new_node_idx);
      MORUM_TRACE("new branch #{} parent #{}'s child {}",
          new_node_idx,
          current_branch,
          old_key_bits[i]);
      current_branch = new_node_idx;
    }
    auto &current_as_branch = nodes_->get(current_branch)->as_branch();
    current_as_branch.set_child(0, left_idx);
    current_as_branch.set_child(1, right_idx);
    MORUM_TRACE("set branch #{} children to #{} and #{}",
        current_branch,
        left_idx,
        right_idx);
  }
}  // namespace morum
