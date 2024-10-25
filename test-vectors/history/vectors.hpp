/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../vectors.hpp"
#include "types.scale.hpp"

namespace jam::test_vectors_history {
  struct Vectors : test_vectors::VectorsT<TestCase, Config> {
    Vectors() : VectorsT{Config{}} {
      this->list("history/data");
    }
  };
}  // namespace jam::test_vectors_history
