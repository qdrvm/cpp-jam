/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "mock/app/configuration_mock.hpp"
#include "storage/rocksdb/rocksdb.hpp"
#include "testutil/storage/base_fs_test.hpp"

namespace test {

  struct BaseRocksDB_Test : public BaseFS_Test {
    using RocksDB = jam::storage::RocksDb;
    using Buffer = qtils::ByteVec;
    using BufferView = qtils::ByteView;

    BaseRocksDB_Test(fs::path path);

    void open();

    void SetUp() override;

    void TearDown() override;

    std::shared_ptr<jam::log::LoggingSystem> logsys;
    std::shared_ptr<jam::app::ConfigurationMock> app_config;

    std::shared_ptr<RocksDB> rocks_;
    std::shared_ptr<jam::storage::BufferStorage> db_;
  };

}  // namespace test
