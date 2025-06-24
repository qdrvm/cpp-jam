/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */


#include "jam_types/peer_address.hpp"

#include <charconv>

namespace jam {

  outcome::result<PeerAddress> PeerAddress::fromString(std::string_view str) {
    auto at = str.find('@');
    if (at == std::string_view::npos) {
      return Error::MissingAt;
    }

    auto colon = str.find(':', at + 1);
    if (colon == std::string_view::npos) {
      return Error::MissingColon;
    }

    auto peer_id_str = str.substr(0, at);
    auto ip_str = str.substr(at + 1, colon - at - 1);
    auto port_str = str.substr(colon + 1);

    OUTCOME_TRY(id, PeerId::fromString(peer_id_str));

    uint16_t port = 0;
    auto [ptr, ec] = std::from_chars(
        port_str.data(), port_str.data() + port_str.size(), port);
    if (ec != std::errc{}) {
      return Error::InvalidPort;
    }

    return PeerAddress{.id = id, .host = std::string(ip_str), .port = port};
  }

}  // namespace jam


OUTCOME_CPP_DEFINE_CATEGORY(jam, PeerAddress::Error, e) {
  using E = jam::PeerAddress::Error;
  switch (e) {
    case E::Empty:
      return "PeerAddress is empty";
    case E::MissingAt:
      return "Missing '@' in PeerAddress";
    case E::MissingColon:
      return "Missing ':' in PeerAddress";
    case E::InvalidPort:
      return "Invalid port in PeerAddress";
    case E::Malformed:
      return "PeerAddress is malformed";
  }
  __builtin_unreachable();
  return "Unknown PeerAddress::Error";
}
