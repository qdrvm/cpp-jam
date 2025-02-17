/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/enum_error_code.hpp>

namespace jam::snp {
  enum class OpenSslError : uint8_t {
    EVP_PKEY_get_raw_public_key,
    EVP_PKEY_new_raw_private_key,
    SSL_CTX_set_alpn_protos,
    SSL_CTX_use_certificate,
    SSL_CTX_use_PrivateKey,
    SSL_CTX_set_signing_algorithm_prefs,
    SSL_CTX_set_verify_algorithm_prefs,
    SSL_get_peer_certificate,
    X509_gmtime_adj,
    X509_set_pubkey,
    X509_get_pubkey,
    X509_sign,
  };
  Q_ENUM_ERROR_CODE(OpenSslError) {
    using E = decltype(e);
    switch (e) {
      case E::EVP_PKEY_get_raw_public_key:
        return "EVP_PKEY_get_raw_public_key";
      case E::EVP_PKEY_new_raw_private_key:
        return "EVP_PKEY_new_raw_private_key";
      case E::SSL_CTX_set_alpn_protos:
        return "SSL_CTX_set_alpn_protos";
      case E::SSL_CTX_use_certificate:
        return "SSL_CTX_use_certificate";
      case E::SSL_CTX_use_PrivateKey:
        return "SSL_CTX_use_PrivateKey";
      case E::SSL_CTX_set_signing_algorithm_prefs:
        return "SSL_CTX_set_signing_algorithm_prefs";
      case E::SSL_CTX_set_verify_algorithm_prefs:
        return "SSL_CTX_set_verify_algorithm_prefs";
      case E::SSL_get_peer_certificate:
        return "SSL_get_peer_certificate";
      case E::X509_gmtime_adj:
        return "X509_gmtime_adj";
      case E::X509_set_pubkey:
        return "X509_set_pubkey";
      case E::X509_get_pubkey:
        return "X509_get_pubkey";
      case E::X509_sign:
        return "X509_sign";
    }
  }

  enum class LsQuicError : uint8_t {
    lsquic_conn_make_stream,
    lsquic_engine_connect,
    lsquic_engine_new,
    lsquic_global_init,
  };
  Q_ENUM_ERROR_CODE(LsQuicError) {
    using E = decltype(e);
    switch (e) {
      case E::lsquic_conn_make_stream:
        return "lsquic_conn_make_stream";
      case E::lsquic_engine_connect:
        return "lsquic_engine_connect";
      case E::lsquic_engine_new:
        return "lsquic_engine_new";
      case E::lsquic_global_init:
        return "lsquic_global_init";
    }
  }

  enum class ConnectionsError : uint8_t {
    CONNECTION_OPEN_CLOSED,
    CONNECTION_OPEN_DUPLICATE,
    CONNECTIONS_INIT,
    ENGINE_CONNECT_ALREADY,
    ENGINE_CONNECT_CLOSED,
    ENGINE_CONNECT_KEY_MISMATCH,
    ENGINE_OPEN_STREAM_ALREADY,
    ENGINE_OPEN_STREAM_TOO_MANY,
    HANDSHAKE_FAILED,
    PROTOCOL_ID_MAKE_INVALID,
    STREAM_READ_CLOSED,
    STREAM_READ_DESTROYED,
    STREAM_READ_PROTOCOL_ID_CLOSED,
    STREAM_READ_TOO_BIG,
    STREAM_WRITE_CLOSED,
    STREAM_WRITE_DESTROYED,
  };
  Q_ENUM_ERROR_CODE(ConnectionsError) {
    using E = decltype(e);
    switch (e) {
      case E::CONNECTION_OPEN_CLOSED:
        return "Connection::open closed";
      case E::CONNECTION_OPEN_DUPLICATE:
        return "Connection::open duplicate";
      case E::CONNECTIONS_INIT:
        return "Connections::init error";
      case E::ENGINE_CONNECT_ALREADY:
        return "Engine::connect already";
      case E::ENGINE_CONNECT_CLOSED:
        return "Engine::connect closed";
      case E::ENGINE_CONNECT_KEY_MISMATCH:
        return "Engine::connect key mismatch";
      case E::ENGINE_OPEN_STREAM_ALREADY:
        return "Engine::openStream already";
      case E::ENGINE_OPEN_STREAM_TOO_MANY:
        return "Engine::openStream too many streams";
      case E::HANDSHAKE_FAILED:
        return "handshake failed";
      case E::PROTOCOL_ID_MAKE_INVALID:
        return "ProtocolId::make invalid";
      case E::STREAM_READ_CLOSED:
        return "Stream::read closed";
      case E::STREAM_READ_DESTROYED:
        return "Stream::read destroyed";
      case E::STREAM_READ_PROTOCOL_ID_CLOSED:
        return "Stream::readProtocolId closed";
      case E::STREAM_READ_TOO_BIG:
        return "Stream::read too big";
      case E::STREAM_WRITE_CLOSED:
        return "Stream::write closed";
      case E::STREAM_WRITE_DESTROYED:
        return "Stream::write destroyed";
    }
  }
}  // namespace jam::snp
