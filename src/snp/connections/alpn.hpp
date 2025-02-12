/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

#include <qtils/outcome.hpp>

#include "types/genesis_hash.hpp"

struct ssl_ctx_st;
struct ssl_st;

namespace jam::snp {
  // https://github.com/zdave-parity/jam-np/blob/5d374b53578cdd93646e3ee19e2b19ea132317b8/simple.md?plain=1#L30-L41
  class Alpn {
    static constexpr auto kVersion = 0;

   public:
    Alpn(const GenesisHash &genesis);

    outcome::result<void> set(ssl_ctx_st *ssl_ctx);

   private:
    static int select(ssl_st *ssl,
                      const unsigned char **out,
                      unsigned char *outlen,
                      const unsigned char *in,
                      unsigned int inlen,
                      void *void_self);

    qtils::Bytes bytes_;
  };
}  // namespace jam::snp
