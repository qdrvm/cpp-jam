/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

// #include <schnorrkel_crust.h>  // Временно отключено
#include <qtils/bytes.hpp>

// Заглушки для констант из schnorrkel_crust.h
#define ED25519_SECRET_KEY_LENGTH 32
#define ED25519_PUBLIC_KEY_LENGTH 32
#define ED25519_KEYPAIR_LENGTH 64
#define ED25519_SIGNATURE_LENGTH 64

// Заглушки для типов и функций
enum ED25519_RESULT {
    ED25519_RESULT_OK = 0,
    ED25519_RESULT_ERROR = 1
};

inline ED25519_RESULT ed25519_sign(
    uint8_t *signature_out, const uint8_t *keypair, const uint8_t *message, size_t message_length) {
    return ED25519_RESULT_ERROR; // Заглушка
}

inline ED25519_RESULT ed25519_verify(
    const uint8_t *signature, const uint8_t *public_key, const uint8_t *message, size_t message_length) {
    return ED25519_RESULT_ERROR; // Заглушка
}

namespace jam::crypto::ed25519 {
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
}  // namespace jam::crypto::ed25519
