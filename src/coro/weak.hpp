/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "coro/coro.hpp"

#define _CORO_WEAK_AWAIT(tmp_weak, tmp_coro, auto_r, r, shared, coro, ...) \
  ({                                                                       \
    auto tmp_weak = std::weak_ptr{shared};                                 \
    auto tmp_coro = (coro);                                                \
    shared.reset();                                                        \
    auto_r co_await std::move(tmp_coro);                                   \
    shared = tmp_weak.lock();                                              \
    if (not shared) co_return __VA_ARGS__;                                 \
    r                                                                      \
  })
#define CORO_WEAK_AWAIT(shared, coro, ...)                                   \
  _CORO_WEAK_AWAIT(                                                          \
      QTILS_UNIQUE_NAME(tmp_weak), QTILS_UNIQUE_NAME(tmp_coro), auto r =, r; \
      , shared, coro, __VA_ARGS__)

#define CORO_WEAK_AWAIT_V(shared, coro, ...)    \
  _CORO_WEAK_AWAIT(QTILS_UNIQUE_NAME(tmp_weak), \
                   QTILS_UNIQUE_NAME(tmp_coro), \
                   ,                            \
                   ,                            \
                   shared,                      \
                   coro,                        \
                   __VA_ARGS__)
