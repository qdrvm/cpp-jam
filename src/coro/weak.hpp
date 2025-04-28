/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "coro/coro.hpp"

/**
 * Converts shared pointer to weak pointer for `co_await` duration.
 * Shared pointer owner can cancel operation by destroying shared pointer.
 * Checks that state wasn't destroyed during `co_await` before continuing.
 *   auto cb2 = [cb, tmp_weak = std::weak_ptr{shared}, ...](auto r) {
 *     auto shared = tmp_weak.lock();
 *     if (not shared) {
 *       return cb(...);
 *     }
 *     ...
 *     cb();
 *   };
 * Should be used on each level, or use mutable reference
 *   struct Foo {
 *     Coro readMany(Self self) {
 *       CORO_WEAK_AWAIT(self, readOne(self))
 *     }
 *     Coro readOne(Self self) {
 *       CORO_WEAK_AWAIT(self, ...)
 *     }
 *   }
 *   struct Foo {
 *     Coro readMany(Self self) {
 *       co_await readOne(self)
 *     }
 *     Coro readOne(Self &self) {
 *       CORO_WEAK_AWAIT(self, ...)
 *     }
 *   }
 */
#define _CORO_WEAK_AWAIT(tmp_weak, tmp_coro, auto_r, r, shared, coro, ...) \
  ({                                                                       \
    auto tmp_weak = std::weak_ptr{shared};                                 \
    /* coroutine constructor may need `shared` alive */                    \
    auto tmp_coro = (coro);                                                \
    /* reset `shared` after coroutine is constructed */                    \
    shared.reset();                                                        \
    auto_r co_await std::move(tmp_coro);                                   \
    shared = tmp_weak.lock();                                              \
    if (not shared) {                                                      \
      co_return __VA_ARGS__;                                               \
    }                                                                      \
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
