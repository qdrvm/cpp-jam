/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "snp/connections/dns_name.hpp"

#include <cppcodec/detail/base32.hpp>
#include <cppcodec/detail/codec.hpp>

namespace jam::snp::base32 {
  constexpr auto kAlphabet = "abcdefghijklmnopqrstuvwxyz234567";

  struct Config {
    template <typename Codec>
    using codec_impl = cppcodec::detail::stream_codec<Codec, Config>;

    static constexpr size_t alphabet_size() {
      return 32;
    }
    static constexpr char symbol(cppcodec::detail::alphabet_index_t idx) {
      return kAlphabet[idx];
    }
    static constexpr bool generates_padding() {
      return false;
    }
  };

  inline void encode(std::span<char> out, qtils::BytesIn bytes) {
    using codec = cppcodec::detail::codec<cppcodec::detail::base32<Config>>;
    codec::encode(out.data(), out.size(), bytes);
  }
}  // namespace jam::snp::base32

namespace jam::snp {
  DnsName::DnsName(const Key &key) {
    chars[0] = 'e';
    base32::encode(std::span{chars}.subspan(1), key);
  }

  outcome::result<void> DnsName::set(x509_st *x509) const {
    // TODO(turuslan): cert.alt = DnsName(key)
    return outcome::success();
  }
}  // namespace jam::snp
