/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <array>
#include <exception>

#include <qtils/test/outcome.hpp>

#include "testutil/storage/base_rocksdb_test.hpp"

#include "storage/rocksdb/rocksdb.hpp"
#include "storage/storage_error.hpp"

using namespace jam::storage;
namespace fs = std::filesystem;

struct RocksDb_Integration_Test : public test::BaseRocksDB_Test {
  RocksDb_Integration_Test()
      : test::BaseRocksDB_Test("/tmp/kagome_rocksdb_integration_test") {}

  Buffer key_{1, 3, 3, 7};
  Buffer value_{1, 2, 3};
};

/**
 * @given opened database, with {key}
 * @when read {key}
 * @then {value} is correct
 */
TEST_F(RocksDb_Integration_Test, Put_Get) {
  ASSERT_OUTCOME_SUCCESS(db_->put(key_, BufferView{value_}));
  ASSERT_OUTCOME_SUCCESS(contains, db_->contains(key_));
  EXPECT_TRUE(contains);
  ASSERT_OUTCOME_SUCCESS(val, db_->get(key_));
  EXPECT_EQ(val, value_);
}

/**
 * @given empty db
 * @when read {key}
 * @then get "not found"
 */
TEST_F(RocksDb_Integration_Test, Get_NonExistent) {
  ASSERT_OUTCOME_SUCCESS(contains, db_->contains(key_));
  EXPECT_FALSE(contains);
  ASSERT_OUTCOME_SUCCESS(db_->remove(key_));
  ASSERT_OUTCOME_ERROR(db_->get(key_), StorageError::NOT_FOUND);
}

/**
 * @given database with [(i,i) for i in range(6)]
 * @when create batch and write KVs
 * @then data is written only after commit
 */
TEST_F(RocksDb_Integration_Test, WriteBatch) {
  std::list<Buffer> keys{{0}, {1}, {2}, {3}, {4}, {5}};
  Buffer toBeRemoved = {3};
  std::list<Buffer> expected{{0}, {1}, {2}, {4}, {5}};

  auto batch = db_->batch();
  ASSERT_TRUE(batch);

  for (const auto &item : keys) {
    ASSERT_OUTCOME_SUCCESS(batch->put(item, BufferView{item}));
    ASSERT_OUTCOME_SUCCESS(contains, db_->contains(item));
    EXPECT_FALSE(contains);
  }
  ASSERT_OUTCOME_SUCCESS(batch->remove(toBeRemoved));
  ASSERT_OUTCOME_SUCCESS(batch->commit());

  for (const auto &item : expected) {
    ASSERT_OUTCOME_SUCCESS(contains, db_->contains(item));
    EXPECT_TRUE(contains);
    ASSERT_OUTCOME_SUCCESS(val, db_->get(item));
    EXPECT_EQ(val, item);
  }

  ASSERT_OUTCOME_SUCCESS(contains, db_->contains(toBeRemoved));
  EXPECT_FALSE(contains);
}

/**
 * @given database with [(i,i) for i in range(100)]
 * @when iterate over kv pairs forward and backward
 * @then we iterate over all items
 */
TEST_F(RocksDb_Integration_Test, Iterator) {
  const size_t size = 100;
  // 100 buffers of size 1 each; 0..99
  std::list<Buffer> keys;
  for (size_t i = 0; i < size; i++) {
    keys.emplace_back(1, i);
  }

  for (const auto &item : keys) {
    ASSERT_OUTCOME_SUCCESS(db_->put(item, BufferView{item}));
  }

  std::array<size_t, size> counter{};

  logger->warn("forward iteration");
  auto it = db_->cursor();
  ASSERT_OUTCOME_SUCCESS(it->seekFirst());
  for (; it->isValid(); it->next().assume_value()) {
    auto k = it->key().value();
    auto v = it->value().value();
    EXPECT_EQ(k, v);

    logger->info("key: {}, value: {}", k.toHex(), v.view().toHex());

    EXPECT_GE(k[0], 0);
    EXPECT_LT(k[0], size);
    EXPECT_GT(k.size(), 0);

    counter[k[0]]++;
  }

  for (size_t i = 0; i < size; i++) {
    EXPECT_EQ(counter[i], 1);
  }
}
