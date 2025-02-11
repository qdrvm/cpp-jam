/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <jam/snp/connections/protocol_id.hpp>

#include <jam/snp/connections/error.hpp>

namespace jam::snp {
  outcome::result<ProtocolId> ProtocolId::make(Id id, bool unique) {
    ProtocolId protocol_id{id};
    if (unique != protocol_id.unique()) {
      return ConnectionsError::PROTOCOL_ID_MAKE_INVALID;
    }
    return protocol_id;
  }
}  // namespace jam::snp
