/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../vectors.hpp"
#include "types.scale.hpp"

namespace jam::test_vectors_safrole {
  template <bool is_full>
  struct Vectors : test_vectors::VectorsT<TestCase, Config> {
    static constexpr std::string_view type = is_full ? "full" : "tiny";

    Vectors() : VectorsT{is_full ? config_full : config_tiny} {
      this->list(std::filesystem::path{"safrole"} / type);
    }
  };
}  // namespace jam::test_vectors_safrole
