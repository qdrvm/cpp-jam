/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

#include <qtils/enum_error_code.hpp>
#include <qtils/outcome.hpp>

#include "jam_types/peer_id.hpp"

namespace jam {

  struct PeerAddress {
    enum class Error {
      Empty,
      InvalidPort,
      MissingAt,
      MissingColon,
      Malformed,
    };

    PeerId id;
    std::string host;
    uint16_t port;

    static outcome::result<PeerAddress> fromString(std::string_view str);

    friend auto to_string(const PeerAddress &a) -> std::string {
      return fmt::format("{}@{}:{}", to_string(a.id), a.host, a.port);
    }

    auto operator<=>(const PeerAddress &) const = default;
  };

}  // namespace jam

OUTCOME_HPP_DECLARE_ERROR(jam, PeerAddress::Error);

template <>
struct fmt::formatter<jam::PeerAddress> : fmt::formatter<jam::PeerId> {
  auto format(const jam::PeerAddress &peer_address, format_context &ctx) const
      -> decltype(ctx.out()) {
    auto out = ctx.out();
    [[unlikely]] if (long_form) {
      out = fmt::format_to(out,
                           "{:l}@{}:{}",
                           peer_address.id,
                           peer_address.host,
                           peer_address.port);
    } else {
      out = fmt::format_to(out,
                           "{:s}@{}:{}",
                           peer_address.id,
                           peer_address.host,
                           peer_address.port);
    }
    return out;
  }
};
