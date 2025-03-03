/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <stdexcept>
#include <unordered_map>
#include <variant>

namespace qtils {
  template <typename M>
  struct MapEntry {
    using I = typename M::iterator;
    using K = typename M::key_type;

    MapEntry(M &map, const K &key) : map{map} {
      if (auto it = map.find(key); it != map.end()) {
        it_or_key = it;
      } else {
        it_or_key = key;
      }
    }

    bool has() const {
      return std::holds_alternative<I>(it_or_key);
    }
    operator bool() const {
      return has();
    }
    auto &operator*() {
      if (not has()) {
        throw std::logic_error{
            "Call dereference operator of MapEntry without valid iterator"};
      }
      return std::get<I>(it_or_key)->second;
    }
    auto *operator->() {
      if (not has()) {
        throw std::logic_error{
            "Call member access through pointer operator of MapEntry without "
            "valid iterator"};
      }
      return &std::get<I>(it_or_key)->second;
    }
    void insert(M::mapped_type value) {
      if (has()) {
        throw std::logic_error{"MapEntry::insert"};
      }
      it_or_key =
          map.emplace(std::move(std::get<K>(it_or_key)), std::move(value))
              .first;
    }
    void insert_or_assign(M::mapped_type value) {
      if (not has()) {
        insert(std::move(value));
      } else {
        **this = std::move(value);
      }
    }
    /// Remove from map and return value.
    M::mapped_type extract() {
      if (not has()) {
        throw std::logic_error{"MapEntry::extract"};
      }
      auto node = map.extract(std::get<I>(it_or_key));
      it_or_key = std::move(node.key());
      return std::move(node.mapped());
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    M &map;
    std::variant<I, K> it_or_key{};
  };

  template <typename K, typename V, typename L, typename A>
  auto entry(std::map<K, V, L, A> &map, const K &key) {
    return MapEntry<std::remove_cvref_t<decltype(map)>>{map, key};
  }

  template <typename K, typename V, typename H, typename E, typename A>
  auto entry(std::unordered_map<K, V, H, E, A> &map, const K &key) {
    return MapEntry<std::remove_cvref_t<decltype(map)>>{map, key};
  }
}  // namespace qtils
