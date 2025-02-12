/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

#include <TODO_qtils/std_hash_of.hpp>
#include <qtils/outcome.hpp>

namespace jam::snp {
  class Stream;
}  // namespace jam::snp

namespace jam::snp {
  // https://github.com/zdave-parity/jam-np/blob/5d374b53578cdd93646e3ee19e2b19ea132317b8/simple.md?plain=1#L87-L101
  class ProtocolId {
    friend Stream;

    using Id = uint8_t;

    ProtocolId(Id id) : id_{id} {}

   public:
    /**
     * Construct protocol with specified `id`.
     * Check expected `unique` consistency with `id` range.
     */
    static outcome::result<ProtocolId> make(Id id, bool unique);

    auto &id() const {
      return id_;
    }

    bool unique() const {
      return id() < 128;
    }

    auto operator<=>(const ProtocolId &) const = default;

   private:
    Id id_;
  };
}  // namespace jam::snp

template <>
struct std::hash<jam::snp::ProtocolId> {
  size_t operator()(const jam::snp::ProtocolId &v) const {
    return qtils::stdHashOf(v.id());
  }
};
