/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <expected>
#include <iostream>
#include <print>
#include <source_location>
#include <vector>

#include <qtils/bytes.hpp>

#ifdef MORUM_ENABLE_TRACE

#define MORUM_TRACE(msg, ...)                               \
  std::println("{}:{}: {}",                                 \
               std::source_location::current().file_name(), \
               std::source_location::current().line(),      \
               std::format(msg __VA_OPT__(, ) __VA_ARGS__));

namespace morum {
  template <char... Name>
  struct ScopeTimer {
    using Clock = std::chrono::steady_clock;

    ~ScopeTimer() {
      if (!over) {
        auto now = Clock::now();
        MORUM_TRACE("Scope {} ended: {}", Name..., now - start);
      }
    }

    void tick() {
      auto now = Clock::now();
      MORUM_TRACE("Scope {} ticked: {}", Name..., now - start);
    }

    void end() {
      auto now = Clock::now();
      MORUM_TRACE("Scope {} ended: {}", Name..., now - start);
      over = true;
    }

    Clock::time_point start;
    bool over;
  };
}  // namespace morum

#define MORUM_START_SCOPE(name) ScopeTimer<name> morum_##name##_scope_timer_;
#define MORUM_TICK_SCOPE(name) morum_##name##_scope_timer_.tick();
#define MORUM_END_SCOPE(name) morum_##name##_scope_timer_.end();

#else

#define MORUM_TRACE(...)
#define MORUM_START_SCOPE(...)
#define MORUM_TICK_SCOPE(...)
#define MORUM_END_SCOPE(...)

#endif

namespace morum {

  using ByteVector = std::vector<unsigned char>;

  template <size_t N>
  using ByteArray = std::array<unsigned char, N>;

  using Hash32 = ByteArray<32>;

  constexpr Hash32 operator~(Hash32 h) {
    Hash32 res;
    for (size_t i = 0; i < h.size(); ++i) {
      res[i] = ~h[i];
    }
    return res;
  }

  struct StorageError {
    std::string message;
    std::source_location origin = std::source_location::current();
  };

}  // namespace morum

template <>
struct std::formatter<morum::StorageError, char> {
  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    auto it = ctx.begin();
    return it;
  }

  template <class FmtContext>
  FmtContext::iterator format(const morum::StorageError &e,
                              FmtContext &ctx) const {
    auto out = ctx.out();
    std::format_to(out,
                   "From {}:{} - {}\n",
                   e.origin.file_name(),
                   e.origin.line(),
                   e.message);
    return out;
  }
};

template <>
struct std::hash<morum::Hash32> {
  std::size_t operator()(const morum::Hash32 &s) const noexcept {
    // good enough for hash maps, not meant for security
    std::size_t res;
    std::copy_n(s.data(), sizeof(res), reinterpret_cast<uint8_t *>(&res));
    return res;
  }
};

namespace morum {

  inline constexpr const Hash32 ZeroHash32{{}};

  Hash32 blake2b_256(qtils::ByteSpan bytes);

  inline unsigned char reverse_bits(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
  }
}  // namespace morum