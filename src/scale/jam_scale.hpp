/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/config-tiny.hpp>
#include <scale/scale.hpp>

namespace jam {
  template <typename T, typename... Configs>
  [[nodiscard]] outcome::result<std::vector<uint8_t>> encode_with_config(
      T &&value, Configs &&...configs) {
    std::vector<uint8_t> out;
    scale::backend::ToBytes encoder(out, std::forward<Configs>(configs)...);
    try {
      ::scale::encode(std::forward<T>(value), encoder);
    } catch (std::system_error &e) {
      return outcome::failure(e.code());
    }
    return std::move(out);
  }

  template <typename T, typename... Configs>
  [[nodiscard]] outcome::result<T> decode_with_config(const auto &bytes,
                                                      Configs &&...configs) {
    scale::backend::FromBytes decoder(bytes, std::forward<Configs>(configs)...);
    T value;
    try {
      ::scale::decode(value, decoder);
    } catch (std::system_error &e) {
      return outcome::failure(e.code());
    }
    return std::move(value);
  }

#define JAM_DATA_STRUCTURES_CONFIG tiny

#ifdef JAM_DATA_STRUCTURES_CONFIG
  template <typename T>
  auto encode(T &&value) {
    return encode_with_config(  //
        std::forward<T>(value),
        test_vectors::config::JAM_DATA_STRUCTURES_CONFIG);
  }

  template <typename T>
  auto decode(const auto &bytes) {
    return decode_with_config<T>(
        std::forward<decltype(bytes)>(bytes),
        test_vectors::config::JAM_DATA_STRUCTURES_CONFIG);
  }
#else
  using scale::impl::memory::decode;
  using scale::impl::memory::encode;
#endif


}  // namespace jam
