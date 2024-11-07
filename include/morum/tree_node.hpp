/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <utility>

#include <qtils/bytes.hpp>
#include <qtils/cxx/forward_like.hpp>
#include <qtils/optional_ref.hpp>

#include <morum/common.hpp>

namespace morum {

  struct TreeNode;

  // an identifier for a tree node in memory
  using NodeId = uint64_t;

  /**
   * An extended tree branch, where a child may be stored as either a subtree
   * hashe or a NodeId. Children stored by NodeId are supposedly loaded to RAM
   * because they are located on modified paths and need to be re-hashed.
   */
  struct Branch {
    friend class MerkleTree;
    friend TreeNode deserialize_node(qtils::ByteSpan bytes);

    static constexpr NodeId NoId = static_cast<NodeId>(-1);
    static constexpr Hash32 NoHash = []() {
      auto h = ~ZeroHash32;
      h[0] &= 0xFE;  // first bit is always zero for branch nodes
      return h;
    }();

    explicit Branch(const Hash32 &left, std::nullopt_t)
        : left_hash{left}, right_hash{NoHash}, left_idx{NoId}, right_idx{NoId} {
      left_hash[0] &= 0xFE;  // the first bit in a branch must be zero
    }

    explicit Branch(std::nullopt_t, const Hash32 &right)
        : left_hash{NoHash},
          right_hash{right},
          left_idx{NoId},
          right_idx{NoId} {}

    explicit Branch(NodeId left, std::nullopt_t)
        : left_hash{NoHash},
          right_hash{NoHash},
          left_idx{left},
          right_idx{NoId} {}

    explicit Branch(std::nullopt_t, NodeId right)
        : left_hash{NoHash},
          right_hash{NoHash},
          left_idx{NoId},
          right_idx{right} {}

    explicit Branch(const Hash32 &left, const Hash32 &right)
        : left_hash{left}, right_hash{right}, left_idx{NoId}, right_idx{NoId} {
      left_hash[0] &= 0xFE;  // the first bit in a branch must be zero
    }

    explicit Branch(qtils::OptionalRef<const Hash32> left,
        qtils::OptionalRef<const Hash32> right)
        : left_hash{left ? *left : NoHash},
          right_hash{right ? *right : NoHash},
          left_idx{NoId},
          right_idx{NoId} {
      if (left_hash != NoHash) {
        left_hash[0] &= 0xFE;  // the first bit in a branch must be zero
      }
    }

    explicit Branch(NodeId left, NodeId right)
        : left_hash{NoHash},
          right_hash{NoHash},
          left_idx{left},
          right_idx{right} {}

    explicit Branch(std::pair<uint8_t, const Hash32 &> with_idx)
        : left_hash{with_idx.first == 0 ? with_idx.second : NoHash},
          right_hash{with_idx.first == 1 ? with_idx.second : NoHash},
          left_idx{NoId},
          right_idx{NoId} {
      left_hash[0] &= 0xFE;  // the first bit in a branch must be zero
    }

    explicit Branch(std::pair<uint8_t, NodeId> with_idx)
        : left_hash{NoHash},
          right_hash{NoHash},
          left_idx{with_idx.first == 0 ? with_idx.second : NoId},
          right_idx{with_idx.first == 1 ? with_idx.second : NoId} {}

    Branch()
        : left_hash{NoHash},
          right_hash{NoHash},
          left_idx{NoId},
          right_idx{NoId} {}

    qtils::OptionalRef<const Hash32> get_left_hash() const {
      if (left_hash != NoHash && left_hash != ZeroHash32) {
        return left_hash;
      }
      return std::nullopt;
    }

    qtils::OptionalRef<const Hash32> get_right_hash() const {
      if (right_hash != NoHash && right_hash != ZeroHash32) {
        return right_hash;
      }
      return std::nullopt;
    }

    const Hash32 &get_left_hash_raw() const {
      return left_hash;
    }

    const Hash32 &get_right_hash_raw() const {
      return right_hash;
    }

    qtils::OptionalRef<const Hash32> get_child_hash(uint8_t bit) const {
      return (bit ? get_right_hash() : get_left_hash());
    }

    std::optional<NodeId> get_left_idx() const {
      if (left_idx != NoId) {
        return left_idx;
      }
      return std::nullopt;
    }

    std::optional<NodeId> get_right_idx() const {
      if (right_idx != NoId) {
        return right_idx;
      }
      return std::nullopt;
    }

    std::optional<NodeId> get_child_idx(uint8_t bit) const {
      return (bit ? get_right_idx() : get_left_idx());
    }

    void set_left(const Hash32 &hash) {
      left_hash = hash;
      left_hash[0] &= 0xFE;  // the first bit in a branch must be zero
    }

    void set_right(const Hash32 &hash) {
      right_hash = hash;
    }

    void set_left(qtils::FixedByteSpan<32> hash) {
      std::ranges::copy(hash, left_hash.begin());
      left_hash[0] &= 0xFE;  // the first bit in a branch must be zero
    }

    void set_right(qtils::FixedByteSpan<32> hash) {
      std::ranges::copy(hash, right_hash.begin());
    }

    void set_child(uint8_t bit, qtils::FixedByteSpan<32> hash) {
      if (bit == 0) {
        set_left(hash);
      } else {
        set_right(hash);
      }
    }

    void reset_child_hash(uint8_t bit) {
      if (bit == 0) {
        left_hash = NoHash;
      } else {
        right_hash = NoHash;
      }
    }

    void set_left(NodeId idx) {
      left_idx = idx;
    }

    void set_right(NodeId idx) {
      right_idx = idx;
    }

    void set_child(uint8_t bit, NodeId idx) {
      if (bit == 0) {
        left_idx = idx;
      } else {
        right_idx = idx;
      }
    }

   private:
    Hash32 left_hash;
    Hash32 right_hash;
    NodeId left_idx;
    NodeId right_idx;
    ByteArray<42>
        padding;  // so that branch's size is a multiple of leaf's size
  };

  using HashRef = std::reference_wrapper<const Hash32>;

  /**
   * A tree leaf
   */
  class Leaf {
   public:
    // values with size <= 32 are not hashed and embedded
    using HashOrValue = std::variant<HashRef, qtils::ByteSpan>;

    enum class Type : unsigned char {
      EmbeddedValue = 0b01,
      HashedValue = 0b11
    };
    template <Type>
    struct Tag {};

    using EmbeddedTag = Tag<Type::EmbeddedValue>;
    using HashedTag = Tag<Type::HashedValue>;

    Leaf(Tag<Type::EmbeddedValue>,
        const ByteArray<31> &key,
        qtils::ByteSpan value)
        : type{Type::EmbeddedValue},
          value_size{static_cast<uint8_t>(value.size_bytes())},
          key{key},
          value{} {
      QTILS_ASSERT_LESS_EQ(value.size_bytes(), 32);
      std::copy_n(value.begin(), value.size_bytes(), this->value.begin());
    }

    Leaf(Tag<Type::HashedValue>,
        const qtils::ByteArray<31> &key,
        qtils::FixedByteSpan<32> value)
        : type{Type::HashedValue}, value_size{}, key{key} {
      std::ranges::copy(value, this->value.begin());
    }

    HashOrValue hash_or_value() const {
      switch (type) {
        case Type::HashedValue:
          return std::ref(value);
        case Type::EmbeddedValue: {
          return qtils::ByteSpan{value.data(), value_size};
        }
      }
      std::unreachable();
    }

    const qtils::ByteArray<32> &raw_value() const {
      return value;
    }

    const qtils::ByteArray<31> &get_key() const {
      return key;
    }

   private:
    Type type : 2;
    uint8_t value_size : 6;

    qtils::ByteArray<31> key;
    qtils::ByteArray<32> value;
  };

  template <std::ranges::random_access_range R>
  uint8_t get_bit(R &&r, size_t bit_idx) {
    if constexpr (std::ranges::sized_range<R>) {
      QTILS_ASSERT_GREATER(std::ranges::size(r) * 8, bit_idx);
    }
    return (r[bit_idx / 8] >> (bit_idx % 8)) & 0x1;
  }

  /**
   * A tree node is either a leaf or a branch.
   */
  struct TreeNode {
   public:
    TreeNode(const Branch &branch) : node{.branch = branch} {}
    TreeNode(const Leaf &leaf) : node{.leaf = leaf} {}

    bool is_branch() const {
      // first bit of a node denotes its type
      return (reinterpret_cast<const uint8_t *>(&node)[0] & 1) == 0;
    }

    bool is_leaf() const {
      // first bit of a node denotes its type
      return (reinterpret_cast<const uint8_t *>(&node)[0] & 1) == 1;
    }
#if defined(__cpp_explicit_this_parameter) \
    && __cpp_explicit_this_parameter >= 202110L

    auto &&as_branch(this auto &self) {
      QTILS_ASSERT(self.is_branch());
      return qtils::cxx23::forward_like<decltype(self)>(self.node.branch);
    }

    auto &&as_leaf(this auto &self) {
      QTILS_ASSERT(self.is_leaf());
      return qtils::cxx23::forward_like<decltype(self)>(self.node.leaf);
    }
#else

    auto &as_branch() {
      QTILS_ASSERT(is_branch());
      return node.branch;
    }

    const auto &as_branch() const {
      QTILS_ASSERT(is_branch());
      return node.branch;
    }

    auto &as_leaf() {
      QTILS_ASSERT(is_leaf());
      return node.leaf;
    }

    const auto &as_leaf() const {
      QTILS_ASSERT(is_leaf());
      return node.leaf;
    }

#endif
   private:
    // first bit of a node denotes its type
    union {
      Leaf leaf;
      Branch branch;
    } node;
  };

  // branches are larger because they store additional information required for
  static_assert(sizeof(Branch) == 128);
  static_assert(sizeof(Leaf) == 64);
  static_assert(sizeof(TreeNode) == 128);

  ByteArray<64> serialize_leaf(
      const ByteArray<31> &key, const Leaf::HashOrValue &value);

  ByteArray<64> serialize_branch(const Hash32 &left, const Hash32 &right);

  TreeNode deserialize_node(qtils::ByteSpan bytes);

}  // namespace morum
