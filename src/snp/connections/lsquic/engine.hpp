/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <deque>
#include <lsquic.h>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>

#include "coro/coro.hpp"
#include "coro/handler.hpp"
#include "coro/io_context_ptr.hpp"
#include "snp/connections/address.hpp"
#include "snp/connections/connection_id_counter.hpp"
#include "snp/connections/connection_info.hpp"
#include "snp/connections/connection_ptr.hpp"
#include "snp/connections/protocol_id.hpp"
#include "snp/connections/stream_ptr.hpp"
#include "snp/connections/tls_certificate.hpp"

struct sockaddr;

namespace jam::snp {
  class ConnectionsConfig;
}  // namespace jam::snp

namespace jam::snp::lsquic {
  class Engine;
  class EngineController;
}  // namespace jam::snp::lsquic

namespace jam::snp::lsquic {
  using Socket = boost::asio::ip::udp::socket;
  Socket::endpoint_type make_endpoint(const Address &address);

  /**
   * Captures `Engine::connect` arguments.
   */
  struct Connecting {
    Address address;
    CoroHandler<ConnectionPtrOutcome> handler;
  };

  /**
   * `lsquic_conn_ctx_t`.
   */
  struct ConnCtx {
    using Ls = lsquic_conn_ctx_t;

    std::weak_ptr<Engine> engine;
    std::optional<lsquic_conn_t *> ls_conn;
    std::optional<std::weak_ptr<Connection>> connection;
    std::optional<Connecting> connecting;
    std::optional<ConnectionInfo> info;
    std::optional<StreamPtr> open_stream;

    bool canDelete() const {
      return not ls_conn.has_value() and not connection.has_value();
    }
  };

  /**
   * `lsquic_stream_ctx_t`.
   */
  struct StreamCtx {
    using Ls = lsquic_stream_ctx_t;

    std::weak_ptr<Engine> engine;
    std::optional<lsquic_stream_t *> ls_stream;
    std::optional<std::weak_ptr<Stream>> stream;
    std::optional<CoroHandler<void>> reading;
    std::optional<CoroHandler<void>> writing;
    bool want_flush = false;

    bool canDelete() const {
      return not ls_stream.has_value() and not stream.has_value();
    }
  };

  class Engine : public std::enable_shared_from_this<Engine> {
    friend Connection;
    friend Stream;

    struct Private {};

   public:
    using SelfSPtr = std::shared_ptr<Engine>;

    static outcome::result<std::shared_ptr<Engine>> make(
        IoContextPtr io_context_ptr,
        ConnectionIdCounter connection_id_counter,
        TlsCertificate certificate,
        std::optional<uint16_t> listen_port,
        std::weak_ptr<EngineController> controller);
    Engine(Private,
           IoContextPtr io_context_ptr,
           ConnectionIdCounter connection_id_counter,
           TlsCertificate &&certificate,
           Socket &&socket,
           Socket::endpoint_type socket_local_endpoint,
           std::weak_ptr<EngineController> controller);
    ~Engine();

    static ConnectionPtrCoroOutcome connect(SelfSPtr self, Address address);

   private:
    struct Reading {
      static constexpr size_t kMaxUdpPacketSize = 64 << 10;
      qtils::BytesN<kMaxUdpPacketSize> buffer;
      boost::asio::ip::udp::endpoint remote_endpoint;
    };

    void wantFlush(StreamCtx *stream_ctx);
    void wantProcess();
    void process();
    void readLoop();
    static void destroyConnection(ConnCtx *conn_ctx);
    static StreamPtrCoroOutcome openStream(ConnCtx *conn_ctx,
                                           ProtocolId protocol_id);
    static void destroyStream(StreamCtx *stream_ctx);
    void streamAccept(StreamPtr &&stream);
    static void streamShutdownRead(StreamCtx *stream_ctx);
    static void streamShutdownWrite(StreamCtx *stream_ctx);
    static CoroOutcome<bool> streamReadRaw(StreamCtx *stream_ctx,
                                           qtils::BytesOut message);
    static CoroOutcome<void> streamWriteRaw(StreamCtx *stream_ctx,
                                            qtils::BytesIn message);

    /**
     * Called from `lsquic_engine_connect` (client),
     * `lsquic_engine_process_conns` (server).
     */
    static lsquic_conn_ctx_t *on_new_conn(void *void_self,
                                          lsquic_conn_t *ls_conn);
    /**
     * Called from `lsquic_engine_process_conns`, `lsquic_engine_destroy`.
     */
    static void on_conn_closed(lsquic_conn_t *ls_conn);
    /**
     * Called from `lsquic_engine_packet_in` (client),
     * `on_new_conn` (server).
     */
    static void on_hsk_done(lsquic_conn_t *ls_conn, lsquic_hsk_status status);
    /**
     * Called from `lsquic_conn_make_stream` (client),
     * `lsquic_engine_process_conns` (server).
     */
    static lsquic_stream_ctx_t *on_new_stream(void *void_self,
                                              lsquic_stream_t *ls_stream);
    /**
     * Called from `lsquic_engine_process_conns`, `lsquic_engine_destroy`.
     */
    static void on_close(lsquic_stream_t *ls_stream,
                         lsquic_stream_ctx_t *ls_stream_ctx);
    /**
     * Called from `lsquic_engine_process_conns`.
     * `lsquic_stream_flush` doesn't work inside `on_read`.
     */
    static void on_read(lsquic_stream_t *ls_stream,
                        lsquic_stream_ctx_t *ls_stream_ctx);
    /**
     * Called from `lsquic_engine_process_conns`.
     */
    static void on_write(lsquic_stream_t *ls_stream,
                         lsquic_stream_ctx_t *ls_stream_ctx);
    /**
     * Called from `lsquic_engine_connect` (client),
     * `lsquic_engine_packet_in` (server).
     */
    static ssl_ctx_st *ea_get_ssl_ctx(void *void_self, const sockaddr *);
    /**
     * Called from `lsquic_engine_process_conns`,
     * `lsquic_engine_send_unsent_packets`.
     */
    static int ea_packets_out(void *void_self,
                              const lsquic_out_spec *out_spec,
                              unsigned n_packets_out);

    IoContextPtr io_context_ptr_;
    ConnectionIdCounter connection_id_counter_;
    TlsCertificate certificate_;
    Socket socket_;
    Socket::endpoint_type socket_local_endpoint_;
    std::weak_ptr<EngineController> controller_;
    boost::asio::steady_timer timer_;
    lsquic_engine_t *engine_ = nullptr;
    Reading reading_;
    std::optional<Connecting> connecting_;
    std::deque<std::weak_ptr<Stream>> want_flush_;
    bool want_process_ = false;
  };
}  // namespace jam::snp::lsquic
