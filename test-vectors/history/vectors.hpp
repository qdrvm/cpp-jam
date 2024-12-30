/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <test-vectors/config-full.hpp>
#include <test-vectors/config-tiny.hpp>
#include <test-vectors/history/history-scale.hpp>
#include <test-vectors/vectors.hpp>

namespace jam::test_vectors::history {
  struct Vectors : test_vectors::VectorsT<TestCase, Config> {
    Vectors() : VectorsT{Config{}} {
      this->list(std::filesystem::path{"history/data"});
    }

    static std::vector<std::shared_ptr<Vectors>> vectors() {
      return {std::make_shared<Vectors>()};
    }
  };
}  // namespace jam::test_vectors_history
