/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <test-vectors/disputes/types.scale.hpp>
#include <test-vectors/vectors.hpp>

namespace jam::test_vectors_disputes {
  template <bool is_full>
  struct Vectors : test_vectors::VectorsT<TestCase, Config> {
    static constexpr std::string_view type = is_full ? "full" : "tiny";

    Vectors() : VectorsT{is_full ? config_full : config_tiny} {
      this->list(std::filesystem::path{"disputes"} / type);
    }
  };
}  // namespace jam::test_vectors_disputes
