/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <cstddef>
#include <stdexcept>

namespace jam {

  class NonCopyable {
   public:
    // To prevent copy of instance
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
  };

  class NonMovable {
   public:
    // To prevent movement of instance
    NonMovable() = default;
    NonMovable(const NonMovable &) = delete;
    NonMovable &operator=(const NonMovable &) = delete;
  };

  class StackOnly {
   public:
    // To prevent an object being created on the heap
    void *operator new(std::size_t) = delete;            // standard new
    void *operator new(std::size_t, void *) = delete;    // placement new
    void *operator new[](std::size_t) = delete;          // array new
    void *operator new[](std::size_t, void *) = delete;  // placement array new
  };

  class HeapOnly {
   protected:
    ~HeapOnly() = default;
  };

  template <typename T>
  class Singleton : public NonCopyable, public NonMovable {
    using BaseType = T;

   public:
    Singleton() {
      if (exists.test_and_set(std::memory_order_acquire)) {
        throw std::logic_error(
            "Attempt to create one more instance for singleton");
      }
    }
    ~Singleton() {
      exists.clear(std::memory_order_release);
    }

   private:
    inline static std::atomic_flag exists{false};
  };

}  // namespace jam
