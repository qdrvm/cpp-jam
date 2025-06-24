/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma omp

#include "jam_types/types.hpp"

namespace jam {

  struct EpochMark {
    Entropy entropy;
    Entropy tickets_entropy;
    std::vector<BandersnatchPublic> validators;
    bool operator==(const EpochMark &) const = default;
  };

}  // namespace jam
