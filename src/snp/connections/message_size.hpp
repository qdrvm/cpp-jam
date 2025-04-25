/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <limits>

namespace jam::snp {
  // https://github.com/zdave-parity/jam-np/blob/5d374b53578cdd93646e3ee19e2b19ea132317b8/simple.md?plain=1#L109-L111
  using MessageSize = uint32_t;
  constexpr MessageSize kMessageSizeMax =
      std::numeric_limits<MessageSize>::max();
}  // namespace jam::snp
