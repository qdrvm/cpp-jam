/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/bytes.hpp>
#include <scale/scale.hpp>

namespace jam {

  template <typename T>
    requires(not std::is_base_of_v<std::decay<T>, scale::ScaleEncoderStream>)
  outcome::result<scale::ByteArray> encode(const T &v, const auto &config) {
    scale::ScaleEncoderStream s(config);
    OUTCOME_TRY(encode(s, v));
    return outcome::success(s.to_vector());
  }

  template <typename T>
    requires(not std::is_base_of_v<std::decay<T>, scale::ScaleDecoderStream>)
  outcome::result<T> decode(qtils::BytesIn bytes, const auto &config) {
    scale::ScaleDecoderStream s(bytes, config);
    T t;
    OUTCOME_TRY(scale::decode<T>(s, t));
    return outcome::success(std::move(t));
  }

}  // namespace scale
