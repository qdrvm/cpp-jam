/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "snp/connections/dns_name.hpp"

namespace jam {
  struct PeerId {
    snp::Key key;
  };
  inline auto format_as(const PeerId &peer_id) {
    return snp::DnsName{peer_id.key};
  }
}  // namespace jam
