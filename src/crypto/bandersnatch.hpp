/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <optional>

// #include <jam_crust.h>  // Временно отключено
#include <qtils/bytes.hpp>

// Определения для замены констант из jam_crust.h
#define JAM_BANDERSNATCH_OUTPUT 32
#define JAM_BANDERSNATCH_PUBLIC 32
#define JAM_BANDERSNATCH_RING_COMMITMENT 32
#define JAM_BANDERSNATCH_RING_SIGNATURE 96

// Заглушки для типов и функций
using JamBandersnatchRing = void;
using JamBandersnatchRingVerifier = void;

inline JamBandersnatchRing* jam_bandersnatch_ring_new(uint32_t) { return nullptr; }
inline void jam_bandersnatch_ring_drop(JamBandersnatchRing*) {}
inline void jam_bandersnatch_ring_commitment(JamBandersnatchRing*, const uint8_t*, size_t, uint8_t*) {}
inline JamBandersnatchRingVerifier* jam_bandersnatch_ring_verifier_new(JamBandersnatchRing*, const uint8_t*) { return nullptr; }
inline void jam_bandersnatch_ring_verifier_drop(JamBandersnatchRingVerifier*) {}
inline bool jam_bandersnatch_ring_verifier_verify(JamBandersnatchRingVerifier*, const uint8_t*, size_t, const uint8_t*, uint8_t*) { return false; }
inline bool jam_bandersnatch_output(const uint8_t*, uint8_t*) { return false; }

namespace jam::crypto::bandersnatch {
  using Output = qtils::BytesN<JAM_BANDERSNATCH_OUTPUT>;
  using Public = qtils::BytesN<JAM_BANDERSNATCH_PUBLIC>;
  using RingCommitment = qtils::BytesN<JAM_BANDERSNATCH_RING_COMMITMENT>;
  using RingSignature = qtils::BytesN<JAM_BANDERSNATCH_RING_SIGNATURE>;

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
}  // namespace jam::crypto::bandersnatch
