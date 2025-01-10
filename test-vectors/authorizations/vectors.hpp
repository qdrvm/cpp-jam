/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <test-vectors/config-full.hpp>
#include <test-vectors/config-tiny.hpp>
#include <test-vectors/authorizations/authorizations-scale.hpp>
#include <test-vectors/vectors.hpp>

namespace jam::test_vectors::authorizations {
  struct Vectors : test_vectors::VectorsT<TestCase, Config> {
    std::string_view type;

    explicit Vectors(bool is_full)
        : VectorsT{is_full ? config::full : config::tiny},
          type{is_full ? "full" : "tiny"} {
      this->list(std::filesystem::path{"authorizations"} / type);
    }

    static std::vector<std::shared_ptr<Vectors>> vectors() {
      return {
          std::make_shared<Vectors>(false),
          std::make_shared<Vectors>(true),
      };
    }
  };
}  // namespace jam::test_vectors::authorizations
