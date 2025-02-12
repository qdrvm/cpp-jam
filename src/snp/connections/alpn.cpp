/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "snp/connections/alpn.hpp"

#include <openssl/ssl.h>
#include <qtils/append.hpp>
#include <qtils/bytestr.hpp>
#include <qtils/hex.hpp>

#include "snp/connections/error.hpp"

namespace jam::snp {
  Alpn::Alpn(const GenesisHash &genesis) {
    const auto protocol =
        fmt::format("jamnp-s/{}/{:x}", kVersion, std::span{genesis}.first(4));
    bytes_.reserve(1 + protocol.size());
    bytes_.emplace_back(protocol.size());
    qtils::append(bytes_, qtils::str2byte(protocol));
  }

  outcome::result<void> Alpn::set(ssl_ctx_st *ssl_ctx) {
    if (SSL_CTX_set_alpn_protos(ssl_ctx, bytes_.data(), bytes_.size()) != 0) {
      return OpenSslError::SSL_CTX_set_alpn_protos;
    }
    SSL_CTX_set_alpn_select_cb(ssl_ctx, select, this);
    return outcome::success();
  }

  int Alpn::select(ssl_st *ssl,
                   const unsigned char **out,
                   unsigned char *outlen,
                   const unsigned char *in,
                   unsigned int inlen,
                   void *void_self) {
    auto *self = static_cast<Alpn *>(void_self);
    uint8_t *out2 = nullptr;
    int r = SSL_select_next_proto(
        &out2, outlen, in, inlen, self->bytes_.data(), self->bytes_.size());
    *out = out2;
    return r == OPENSSL_NPN_NEGOTIATED ? SSL_TLSEXT_ERR_OK
                                       : SSL_TLSEXT_ERR_ALERT_FATAL;
  }
}  // namespace jam::snp
