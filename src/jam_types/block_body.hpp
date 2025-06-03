/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/constants.hpp"
#include "jam_types/vote.hpp"

namespace jam {

  struct BlockBody {
    /// @note votes will be replaced by aggregated attestations.
    std::array<Vote, VALIDATOR_REGISTRY_LIMIT> votes;
  };

}  // namespace jam
