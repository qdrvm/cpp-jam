/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "jam_types/peer_id.hpp"

#include <qtils/byte_arr.hpp>
#include <qtils/outcome.hpp>

#include "utils/base_codec.hpp"

namespace {
  inline constexpr char ALPHABET[] = "abcdefghijklmnopqrstuvwxyz234567";
  using Codec = jam::utils::BaseCodec<32, ALPHABET>;
}

namespace jam {

  outcome::result<PeerId> PeerId::fromString(std::string_view str) {
    if (str.empty()) {
      return Error::Empty;
    }
    if (str.size() < 53) {
      return Error::TooShort;
    }
    if (str.size() > 53) {
      return Error::TooLong;
    }
    if (str[0] != 'e') {
      return Error::Malformed;
    }
    PeerId peer_id;
    OUTCOME_TRY(Codec::decode_into(str.substr(1), peer_id));
    return peer_id;
  }

  std::string PeerId::str() const {
    return 'e' + Codec::encode(*this);
  }

};  // namespace jam

OUTCOME_CPP_DEFINE_CATEGORY(jam, PeerId::Error, e) {
  using E = jam::PeerId::Error;
  switch (e) {
    case E::Empty:
      return "PeerId is empty";
    case E::TooShort:
      return "PeerId is too short";
    case E::TooLong:
      return "PeerId is too long";
    case E::Malformed:
      return "PeerId is malformed";
  }
  __builtin_unreachable();
  return "Unknown PeerId::Error";
}
