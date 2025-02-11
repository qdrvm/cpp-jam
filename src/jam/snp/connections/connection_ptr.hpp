/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam/coro/coro.hpp>
#include <memory>

namespace jam::snp {
  class Connection;
}  // namespace jam::snp

namespace jam::snp {
  using ConnectionPtr = std::shared_ptr<Connection>;
  using ConnectionPtrOutcome = outcome::result<ConnectionPtr>;
  using ConnectionPtrCoroOutcome = CoroOutcome<ConnectionPtr>;
}  // namespace jam::snp
