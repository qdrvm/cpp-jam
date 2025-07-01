/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/types.tmp.hpp>

namespace jam {

  using test_vectors::TicketsExtrinsic;
  using test_vectors::PreimagesExtrinsic;
  using test_vectors::GuaranteesExtrinsic;
  using test_vectors::AssurancesExtrinsic;
  using test_vectors::DisputesExtrinsic;

  struct BlockBody {
    TicketsExtrinsic tickets;
    PreimagesExtrinsic preimages;
    GuaranteesExtrinsic guarantees;
    AssurancesExtrinsic assurances;
    DisputesExtrinsic disputes;
    bool operator==(const BlockBody &) const = default;

    operator test_vectors::Extrinsic() const {
      return reinterpret_cast<const test_vectors::Extrinsic&>(*this);
    }
  };

}  // namespace jam
