/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/use_awaitable.hpp>

#include "coro/coro.hpp"
#include "coro/io_context_ptr.hpp"

namespace jam {
  inline Coro<void> setCoroThread(IoContextPtr io_context_ptr) {
    if (not io_context_ptr->get_executor().running_in_this_thread()) {
      co_await boost::asio::post(*io_context_ptr, boost::asio::use_awaitable);
    }
  }
}  // namespace jam
