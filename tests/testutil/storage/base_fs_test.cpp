/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "testutil/storage/base_fs_test.hpp"

#include "log/logger.hpp"
#include "testutil/prepare_loggers.hpp"

namespace test {

  void BaseFS_Test::clear() {
    if (fs::exists(base_path)) {
      fs::remove_all(base_path);
    }
  }

  void BaseFS_Test::mkdir() {
    fs::create_directory(base_path);
  }

  std::string BaseFS_Test::getPathString() const {
    return fs::canonical(base_path).string();
  }

  BaseFS_Test::~BaseFS_Test() {
    clear();
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  BaseFS_Test::BaseFS_Test(fs::path path)
      : logger(testutil::prepareLoggers()->createLogger(
            fs::weakly_canonical(path).string(),
            "testing",
            jam::log::Level::DEBUG)),
        base_path(std::move(path)) {
    clear();
    mkdir();
  }
#pragma GCC diagnostic pop

  void BaseFS_Test::SetUp() {
    clear();
    mkdir();
  }

  void BaseFS_Test::TearDown() {
    clear();
  }
}  // namespace test
