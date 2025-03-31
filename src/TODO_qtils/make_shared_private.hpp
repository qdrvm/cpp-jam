/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

namespace qtils {
  /**
   * `enable_shared_from_this` requires `make_shared`,
   * which requires public constructor.
   * `injector` may accidentially call wrong constructor.
   * Using `MakeSharedPrivate` argument in public constructor prevents injector
   * from using it.
   *   class Foo : public std::enable_shared_from_this<Foo> {
   *    public:
   *     Foo(MakeSharedPrivate, ...);
   *     static auto factory(...) {
   *       return MakeSharedPrivate::make(...);
   *     }
   *   };
   */
  class MakeSharedPrivate {
    MakeSharedPrivate() = default;

   public:
    template <typename T, typename... A>
    static std::shared_ptr<T> make(A &&...args) {
      return std::make_shared<T>(MakeSharedPrivate{},
                                 std::forward<decltype(args)>(args)...);
    }
  };
}  // namespace qtils
