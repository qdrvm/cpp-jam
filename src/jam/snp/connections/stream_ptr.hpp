/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam/coro/coro.hpp>
#include <memory>

namespace jam::snp {
  class Stream;
}  // namespace jam::snp

namespace jam::snp {
  using StreamPtr = std::shared_ptr<Stream>;
  using StreamPtrCoroOutcome = CoroOutcome<StreamPtr>;
}  // namespace jam::snp
