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
  struct Vectors
      : test_vectors::Vectors<
            typename std::conditional_t<is_full, full, tiny>::Testcase> {
    using types = std::conditional_t<is_full, full, tiny>;

    std::string type = is_full ? "full" : "tiny";

    Vectors() {
      this->list(std::filesystem::path{"safrole"} / type);
    }
  };
}  // namespace jam::test_vectors_safrole
