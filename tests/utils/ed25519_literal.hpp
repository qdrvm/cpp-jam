/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/unhex.hpp>

#include "crypto/ed25519.hpp"

inline auto operator""_ed25519(const char *c, size_t s) {
  auto seed = qtils::unhex<jam::crypto::ed25519::Seed>({c, s}).value();
  return jam::crypto::ed25519::from_seed(seed);
}
