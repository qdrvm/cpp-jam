/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

namespace boost::asio {
  class io_context;
}  // namespace boost::asio

namespace jam {
  using IoContextPtr = std::shared_ptr<boost::asio::io_context>;
}  // namespace jam
