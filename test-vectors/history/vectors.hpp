/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/config.hpp>
#include <jam_types/history-types.hpp>
#include <test-vectors/vectors.hpp>

namespace morum::test_vectors::history {
  struct Vectors : test_vectors::VectorsT<TestCase, Config> {
    Vectors() : VectorsT{Config{}} {
      this->list(std::filesystem::path{"history/data"});
    }

    static std::vector<std::shared_ptr<Vectors>> vectors() {
      return {std::make_shared<Vectors>()};
    }
  };
}  // namespace morum::test_vectors_history
