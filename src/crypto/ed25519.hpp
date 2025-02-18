/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <schnorrkel_crust.h>

#include <TODO_qtils/from_span.hpp>
#include <qtils/bytes.hpp>

namespace jam::crypto::ed25519 {
  using Seed = qtils::BytesN<ED25519_SEED_LENGTH>;
  using Secret = qtils::BytesN<ED25519_SECRET_KEY_LENGTH>;
  using Public = qtils::BytesN<ED25519_PUBLIC_KEY_LENGTH>;
  using KeyPair = qtils::BytesN<ED25519_KEYPAIR_LENGTH>;
  using Signature = qtils::BytesN<ED25519_SIGNATURE_LENGTH>;
  using Message = qtils::BytesIn;

  inline std::optional<Signature> sign(const KeyPair &keypair,
                                       Message message) {
    Signature sig;
    auto res = ed25519_sign(
        sig.data(), keypair.data(), message.data(), message.size_bytes());
    if (res != ED25519_RESULT_OK) {
      return std::nullopt;
    }
    return sig;
  }

  inline bool verify(const Signature &signature,
                     Message message,
                     const Public &public_key) {
    auto res = ed25519_verify(signature.data(),
                              public_key.data(),
                              message.data(),
                              message.size_bytes());
    return res == ED25519_RESULT_OK;
  }

  inline KeyPair from_seed(const Seed &seed) {
    KeyPair keypair;
    ed25519_keypair_from_seed(keypair.data(), seed.data());
    return keypair;
  }

  inline Public get_public(const KeyPair &keypair) {
    return *qtils::fromSpan<Public>(
        std::span{keypair}.subspan(ED25519_SECRET_KEY_LENGTH));
  }

  inline Public get_secret(const KeyPair &keypair) {
    return *qtils::fromSpan<Secret>(
        std::span{keypair}.first(ED25519_SECRET_KEY_LENGTH));
  }
}  // namespace jam::crypto::ed25519
