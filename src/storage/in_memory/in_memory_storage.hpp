/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <qtils/byte_vec.hpp>
#include <qtils/outcome.hpp>

#include "storage/buffer_map_types.hpp"

namespace jam::storage {

  /**
   * Simple storage that conforms PersistentMap interface
   * Mostly needed to have an in-memory trie in tests to avoid integration with
   * an actual persistent database
   */
  class InMemoryStorage : public storage::BufferStorage {
   public:
    ~InMemoryStorage() override = default;

    outcome::result<ByteVecOrView> get(
        const qtils::ByteView &key) const override;

    outcome::result<std::optional<ByteVecOrView>> tryGet(
        const qtils::ByteView &key) const override;

    outcome::result<void> put(const qtils::ByteView &key,
                              ByteVecOrView &&value) override;

    outcome::result<bool> contains(const qtils::ByteView &key) const override;

    outcome::result<void> remove(const qtils::ByteView &key) override;

    std::unique_ptr<BufferBatch> batch() override;

    std::unique_ptr<Cursor> cursor() override;

    std::optional<size_t> byteSizeHint() const override;

   private:
    std::map<std::string, qtils::ByteVec> storage;
    size_t size_ = 0;

    friend class InMemoryCursor;
  };

}  // namespace jam::storage
