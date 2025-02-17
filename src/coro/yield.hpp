/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/asio/post.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include "coro/coro.hpp"

namespace jam {
  /**
   * Thread switch operation always completes, so it can't leak `shared_ptr`.
   */
  Coro<void> coroYield() {
    co_await boost::asio::post(co_await boost::asio::this_coro::executor,
                               boost::asio::use_awaitable);
  }
}  // namespace jam
