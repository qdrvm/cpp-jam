/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gtest/gtest.h>

#include "log/logger.hpp"
#include "testutil/prepare_loggers.hpp"

// intentionally here, so users can use fs shortcut
namespace fs = std::filesystem;

namespace test {

  /**
   * @brief Base test, which involves filesystem. Can be created with given
   * path. Clears path before test and after test.
   */
  struct BaseFS_Test : public ::testing::Test {
    // not explicit, intentionally
    BaseFS_Test(fs::path path);

    void clear();

    void mkdir();

    std::string getPathString() const;

    ~BaseFS_Test() override;

    void TearDown() override;

    void SetUp() override;

    static void SetUpTestCase() {
      testutil::prepareLoggers();
    }

   protected:
    jam::log::Logger logger;
    fs::path base_path;
  };

}  // namespace test
