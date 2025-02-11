/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <jam/coro/io_context_ptr.hpp>

#define SET_CORO_THREAD(io_context_ptr)                                        \
  ({                                                                           \
    if (not io_context_ptr->get_executor().running_in_this_thread()) {         \
      co_await boost::asio::post(*io_context_ptr, boost::asio::use_awaitable); \
    }                                                                          \
  })
