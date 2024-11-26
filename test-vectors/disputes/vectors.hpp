/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <test-vectors/disputes/types.scale.hpp>
#include <test-vectors/vectors.hpp>

namespace jam::test_vectors_disputes {
  struct Vectors : test_vectors::VectorsT<TestCase, Config> {
    std::string_view type;

    explicit Vectors(bool is_full)
        : VectorsT{is_full ? config_full : config_tiny},
          type{is_full ? "full" : "tiny"} {
      this->list(std::filesystem::path{"disputes"} / type);
    }

    static std::vector<std::shared_ptr<Vectors>> vectors() {
      return {
          std::make_shared<Vectors>(false),
          std::make_shared<Vectors>(true),
      };
    }
  };
}  // namespace jam::test_vectors_disputes
