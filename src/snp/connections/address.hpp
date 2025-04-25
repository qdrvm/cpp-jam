/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "snp/connections/key.hpp"

namespace jam::snp {
  struct Address {
    using Ip = qtils::BytesN<16>;
    static constexpr Ip kLocal{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

    Ip ip;
    uint16_t port;
    Key key;
  };
}  // namespace jam::snp
