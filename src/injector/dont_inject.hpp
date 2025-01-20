/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// Prevent implicit injection.
// Will generate "unresolved symbol" error during linking.
// Not using `= delete` as it would break injector SFINAE.
#define DONT_INJECT(T) explicit T(::jam::injector::DontInjectHelper, ...);

namespace jam::injector {
  struct DontInjectHelper {
    explicit DontInjectHelper() = default;
  };
}  // namespace jam::injector
