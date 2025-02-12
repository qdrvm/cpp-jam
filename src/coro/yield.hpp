/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/asio/post.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

/**
 * Thread switch operation always completes, so it can't leak `shared_ptr`.
 */
#define CORO_YIELD                                                      \
  co_await boost::asio::post(co_await boost::asio::this_coro::executor, \
                             boost::asio::use_awaitable)
