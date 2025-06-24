/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/types.hpp"

namespace jam {

  struct Extrinsic {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    TicketsExtrinsic tickets;
    PreimagesExtrinsic preimages;
    GuaranteesExtrinsic guarantees;
    AssurancesExtrinsic assurances;
    DisputesExtrinsic disputes;
#pragma GCC diagnostic pop
    bool operator==(const Extrinsic &) const = default;
  };

  HAS_PROXY_TEST_VECTORS(Extrinsic);

  TEMPORARY_TYPE_ALIAS(BlockBody, Extrinsic);

}  // namespace jam
