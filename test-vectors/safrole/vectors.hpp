/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/config-full.hpp>
#include <jam_types/config-tiny.hpp>
#include <jam_types/safrole-types.hpp>
#include <test-vectors/vectors.hpp>

namespace jam::test_vectors::safrole {
  struct Vectors : test_vectors::VectorsT<TestCase, Config> {
    std::string_view type;

    explicit Vectors(bool is_full)
        : VectorsT{is_full ? config::full : config::tiny},
          type{is_full ? "full" : "tiny"} {
      this->list(std::filesystem::path{"safrole"} / type);
    }

    static std::vector<std::shared_ptr<Vectors>> vectors() {
      return {
          std::make_shared<Vectors>(false),
          std::make_shared<Vectors>(true),
      };
    }
  };
}  // namespace jam::test_vectors::safrole
