/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

#include <qtils/outcome.hpp>

#include "snp/connections/key.hpp"

struct x509_st;

namespace jam::snp {
  // https://github.com/zdave-parity/jam-np/blob/5d374b53578cdd93646e3ee19e2b19ea132317b8/simple.md?plain=1#L15-L16
  struct DnsName {
    explicit DnsName(const Key &key);

    constexpr operator std::string_view() const {
      return std::string_view{chars.data(), chars.size()};
    }

    /**
     * Set `DnsName` as subject alternative name for certificate.
     */
    outcome::result<void> set(x509_st *x509) const;

    static constexpr size_t kSize = 53;
    std::array<char, kSize> chars;
  };
  constexpr auto format_as(const DnsName &v) {
    return v.operator std::string_view();
  }
}  // namespace jam::snp
