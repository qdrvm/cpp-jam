/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <qtils/outcome.hpp>

#include "snp/connections/key.hpp"

struct ssl_st;

namespace boost::asio::ssl {
  class context;
  class verify_context;
}  // namespace boost::asio::ssl

namespace jam::snp {
  struct ConnectionsConfig;
  class Alpn;
}  // namespace jam::snp

struct ssl_ctx_st;

namespace jam::snp {
  class TlsCertificate {
    TlsCertificate(const ConnectionsConfig &config);

   public:
    /**
     * Generate self-signed tls certificate.
     */
    static outcome::result<TlsCertificate> make(
        const ConnectionsConfig &config);

    /**
     * Allows passing `*this` to openssl functions.
     */
    operator ssl_ctx_st *() const;

    /**
     * Get peer key from tls certificate.
     */
    static outcome::result<Key> get_key(ssl_st *ssl);

   private:
    using Context = boost::asio::ssl::context;

    static bool verify(bool, boost::asio::ssl::verify_context &ctx);

    /**
     * Keeps `Alpn` alive for `SSL_CTX_set_alpn_select_cb`.
     */
    std::shared_ptr<Alpn> alpn_;
    std::shared_ptr<Context> context_;
  };
}  // namespace jam::snp
