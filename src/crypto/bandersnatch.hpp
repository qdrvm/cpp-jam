/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <optional>

#include <ark_vrf/ark_vrf.h>
#include <qtils/byte_arr.hpp>

namespace jam::crypto::bandersnatch {
  using Output = qtils::ByteArr<JAM_BANDERSNATCH_OUTPUT>;
  using Public = qtils::ByteArr<JAM_BANDERSNATCH_PUBLIC>;
  using RingCommitment = qtils::ByteArr<JAM_BANDERSNATCH_RING_COMMITMENT>;
  using RingSignature = qtils::ByteArr<JAM_BANDERSNATCH_RING_SIGNATURE>;

  inline std::optional<Output> output(qtils::BytesIn signature) {
    Output output;
    if (not jam_bandersnatch_output(signature.data(), output.data())) {
      return std::nullopt;
    }
    return output;
  }

  struct RingVerifier;

  struct Ring {
    explicit Ring(uint32_t count)
        : count{count}, ffi{jam_bandersnatch_ring_new(count), Dtor{}} {}

    std::optional<RingCommitment> commitment(
        std::span<const Public> pks) const {
      if (pks.size() != count) {
        return std::nullopt;
      }
      RingCommitment commitment;
      static_assert(sizeof(Public) % alignof(Public) == 0);
      jam_bandersnatch_ring_commitment(ffi.get(),
          pks[0].data(),
          pks.size() * sizeof(Public),
          commitment.data());
      return commitment;
    }

    RingVerifier verifier(const RingCommitment &commitment) const;

    struct Dtor {
      void operator()(JamBandersnatchRing *ptr) const {
        jam_bandersnatch_ring_drop(ptr);
      }
    };

    uint32_t count;
    std::unique_ptr<JamBandersnatchRing, Dtor> ffi;
  };

  struct RingVerifier {
    RingVerifier(const Ring &ring, const RingCommitment &commitment)
        : ffi{
            jam_bandersnatch_ring_verifier_new(
                ring.ffi.get(), commitment.data()),
            Dtor{},
        } {}

    std::optional<Output> verify(
        qtils::BytesIn input, const RingSignature &signature) const {
      Output output;
      if (not jam_bandersnatch_ring_verifier_verify(ffi.get(),
              input.data(),
              input.size(),
              signature.data(),
              output.data())) {
        return std::nullopt;
      }
      return output;
    }

    struct Dtor {
      void operator()(JamBandersnatchRingVerifier *ptr) const {
        jam_bandersnatch_ring_verifier_drop(ptr);
      }
    };

    std::unique_ptr<JamBandersnatchRingVerifier, Dtor> ffi;
  };

  RingVerifier Ring::verifier(const RingCommitment &commitment) const {
    return RingVerifier{*this, commitment};
  }
}  // namespace jam::bandersnatch
