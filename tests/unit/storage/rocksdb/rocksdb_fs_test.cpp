/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "testutil/storage/base_fs_test.hpp"

#include <gtest/gtest.h>

#include <qtils/test/outcome.hpp>

// #include "filesystem/common.hpp"
#include "storage/storage_error.hpp"
#include "storage/rocksdb/rocksdb.hpp"
// #include "testutil/prepare_loggers.hpp"

using namespace jam::storage;
namespace fs = std::filesystem;

struct RocksDb_Open : public test::BaseFS_Test {
  RocksDb_Open() : test::BaseFS_Test("/tmp/jam-test-rocksdb-open") {}
};

/**
 * @given options with disabled option `create_if_missing`
 * @when open database
 * @then database can not be opened (since there is no db already)
 */
TEST_F(RocksDb_Open, OpenNonExistingDB) {
  rocksdb::Options options;
  options.create_if_missing = false;  // intentionally

  auto r = RocksDb::create(logger, getPathString(), options);
  EXPECT_FALSE(r);
  EXPECT_EQ(r.error(), StorageError::INVALID_ARGUMENT);
}

/**
 * @given options with enable option `create_if_missing`
 * @when open database
 * @then database is opened
 */
TEST_F(RocksDb_Open, OpenExistingDB) {
  rocksdb::Options options;
  options.create_if_missing = true;  // intentionally

  ASSERT_OUTCOME_SUCCESS(db, RocksDb::create(logger, getPathString(), options));
  EXPECT_TRUE(db) << "db is nullptr";

  fs::path p(getPathString());
  EXPECT_TRUE(fs::exists(p));
}
