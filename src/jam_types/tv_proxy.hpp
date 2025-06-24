/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <jam_types/config.hpp>
#include <qtils/byte_vec.hpp>
#include <scale/jam_scale.hpp>

namespace jam::test_vectors {

  static Config *current_config = nullptr;

  template <typename From, typename To>
  class Proxy {
   public:
    Proxy(const From &v) : data(&v) {}
    Proxy(const To &v) : data(&v) {}
    Proxy(From &&v) : data(scale::encode(v)) {}
    Proxy(To &&v) : data(scale::encode(v)) {}

    operator From() const {
      if (std::holds_alternative<const From *>(data)) {
        return *std::get<const From *>(data);
      }
      if (std::holds_alternative<qtils::ByteVec>(data)) {
        return scale::decode<From>(std::get<qtils::ByteVec>(data));
      }
      throw std::logic_error("Proxy does not hold a From value");
    }

    operator To() const {
      if (std::holds_alternative<const To *>(data)) {
        return *std::get<const To *>(data);
      }
      if (std::holds_alternative<qtils::ByteVec>(data)) {
        return scale::decode<To>(std::get<qtils::ByteVec>(data));
      }
      throw std::logic_error("Proxy does not hold a To value");
    }

   private:
    std::variant<const From *, const To *, qtils::ByteVec> data;
  };

}  // namespace jam::test_vectors
