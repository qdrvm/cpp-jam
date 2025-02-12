/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "snp/connections/tls_certificate.hpp"

#include <TODO_qtils/asio_buffer.hpp>
#include <TODO_qtils/macro/make_shared.hpp>
#include <boost/asio/ssl/context.hpp>

#include "snp/connections/alpn.hpp"
#include "snp/connections/config.hpp"
#include "snp/connections/dns_name.hpp"
#include "snp/connections/error.hpp"

namespace jam::snp {
  outcome::result<void> set_relative_time(ASN1_TIME *o_time, auto delta) {
    if (not X509_gmtime_adj(
            o_time,
            std::chrono::duration_cast<std::chrono::seconds>(delta).count())) {
      return OpenSslError::X509_gmtime_adj;
    }
    return outcome::success();
  }

  TlsCertificate::TlsCertificate(const ConnectionsConfig &config)
      : MAKE_SHARED_(alpn_, config.genesis),
        MAKE_SHARED_(context_, Context::tlsv13) {}

  outcome::result<TlsCertificate> TlsCertificate::make(
      const ConnectionsConfig &config) {
    TlsCertificate self{config};
    OUTCOME_TRY(self.alpn_->set(self));
    self.context_->set_verify_mode(Context::verify_peer
                                   | Context::verify_fail_if_no_peer_cert
                                   | Context::verify_client_once);
    self.context_->set_verify_callback(verify);
    std::array<uint16_t, 1> prefs{SSL_SIGN_ED25519};
    if (not SSL_CTX_set_signing_algorithm_prefs(
            self, prefs.data(), prefs.size())) {
      return OpenSslError::SSL_CTX_set_signing_algorithm_prefs;
    }
    if (not SSL_CTX_set_verify_algorithm_prefs(
            self, prefs.data(), prefs.size())) {
      return OpenSslError::SSL_CTX_set_verify_algorithm_prefs;
    }
    auto secret = crypto::ed25519::get_secret(config.keypair);
    // `EVP_PKEY_new_raw_private_key` requires seed, but in ed25519 secret=seed
    bssl::UniquePtr<EVP_PKEY> pkey(EVP_PKEY_new_raw_private_key(
        EVP_PKEY_ED25519, nullptr, secret.data(), secret.size()));
    if (not pkey) {
      return OpenSslError::EVP_PKEY_new_raw_private_key;
    }
    if (not SSL_CTX_use_PrivateKey(self, pkey.get())) {
      return OpenSslError::SSL_CTX_use_PrivateKey;
    }

    bssl::UniquePtr<X509> x509(X509_new());
    OUTCOME_TRY(set_relative_time(X509_getm_notBefore(x509.get()),
                                  -std::chrono::days{1}));
    OUTCOME_TRY(set_relative_time(X509_getm_notAfter(x509.get()),
                                  std::chrono::years{1}));
    if (not X509_set_pubkey(x509.get(), pkey.get())) {
      return OpenSslError::X509_set_pubkey;
    }
    OUTCOME_TRY(
        DnsName{crypto::ed25519::get_public(config.keypair)}.set(x509.get()));
    if (not X509_sign(x509.get(), pkey.get(), nullptr)) {
      return OpenSslError::X509_sign;
    }
    if (not SSL_CTX_use_certificate(self, x509.get())) {
      return OpenSslError::SSL_CTX_use_certificate;
    }
    return self;
  }

  TlsCertificate::operator ssl_ctx_st *() const {
    return context_->native_handle();
  }

  outcome::result<Key> TlsCertificate::get_key(ssl_st *ssl) {
    bssl::UniquePtr<X509> x509(SSL_get_peer_certificate(ssl));
    if (not x509) {
      return OpenSslError::SSL_get_peer_certificate;
    }
    bssl::UniquePtr<EVP_PKEY> pkey(X509_get_pubkey(x509.get()));
    if (not pkey) {
      return OpenSslError::X509_get_pubkey;
    }
    Key key;
    size_t key_size = key.size();
    if (not EVP_PKEY_get_raw_public_key(pkey.get(), key.data(), &key_size)) {
      return OpenSslError::EVP_PKEY_get_raw_public_key;
    }
    return key;
  }

  bool TlsCertificate::verify(bool, boost::asio::ssl::verify_context &ctx) {
    X509_STORE_CTX *store_ctx = ctx.native_handle();
    // TODO(turuslan): DnsName(key) == cert.alt
    return true;
  }
}  // namespace jam::snp
