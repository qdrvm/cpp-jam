/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "snp/connections/lsquic/engine.hpp"

#include <TODO_qtils/macro/move.hpp>
#include <TODO_qtils/macro/weak.hpp>
#include <boost/outcome/try.hpp>
#include <qtils/option_take.hpp>

#include "coro/set_thread.hpp"
#include "coro/spawn.hpp"
#include "snp/connections/config.hpp"
#include "snp/connections/connection.hpp"
#include "snp/connections/error.hpp"
#include "snp/connections/lsquic/controller.hpp"
#include "snp/connections/lsquic/init.hpp"
#include "snp/connections/stream.hpp"

#define SELF_FROM_VOID Engine *self = static_cast<Engine *>(void_self)

// TODO(turuslan): unique streams
// TODO(turuslan): connection/stream close event lag

namespace jam::snp::lsquic {
  // TODO(turuslan): config
  constexpr uint32_t kWindowSize = 64 << 10;

  template <typename T>
  T::Ls *to_ls(T *ptr) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<T::Ls *>(ptr);
  }
  template <typename T>
  T *from_ls(typename T::Ls *ptr) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<T *>(ptr);
  }

  void tryDelete(auto *ptr) {
    if (not ptr->canDelete()) {
      return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete ptr;
  }

  Socket::endpoint_type make_endpoint(const Address &address) {
    auto ip = boost::asio::ip::make_address_v6(address.ip);
    return Socket::endpoint_type{ip, address.port};
  }

  outcome::result<std::shared_ptr<Engine>> Engine::make(
      IoContextPtr io_context_ptr,
      ConnectionIdCounter connection_id_counter,
      TlsCertificate certificate,
      std::optional<Port> listen_port,
      std::weak_ptr<EngineController> controller) {
    OUTCOME_TRY(init());

    uint32_t flags = 0;
    if (listen_port) {
      flags |= LSENG_SERVER;
    }

    lsquic_engine_settings settings{};
    lsquic_engine_init_settings(&settings, flags);
    settings.es_init_max_stream_data_bidi_remote = kWindowSize;
    settings.es_init_max_stream_data_bidi_local = kWindowSize;

    static lsquic_stream_if stream_if{};
    stream_if.on_new_conn = on_new_conn;
    stream_if.on_conn_closed = on_conn_closed;
    stream_if.on_hsk_done = on_hsk_done;
    stream_if.on_new_stream = on_new_stream;
    stream_if.on_close = on_close;
    stream_if.on_read = on_read;
    stream_if.on_write = on_write;

    lsquic_engine_api api{};
    api.ea_settings = &settings;

    Socket socket{*io_context_ptr};
    boost::system::error_code ec;
    socket.open(boost::asio::ip::udp::v6(), ec);
    if (ec) {
      return ec;
    }
    socket.non_blocking(true, ec);
    if (ec) {
      return ec;
    }
    if (listen_port) {
      auto ip = boost::asio::ip::address_v6::any();
      socket.bind({ip, listen_port.value()}, ec);
      if (ec) {
        return ec;
      }
    }
    auto socket_local_endpoint = socket.local_endpoint(ec);
    if (ec) {
      return ec;
    }
    auto self = std::make_shared<Engine>(Private{},
                                         io_context_ptr,
                                         std::move(connection_id_counter),
                                         std::move(certificate),
                                         std::move(socket),
                                         socket_local_endpoint,
                                         std::move(controller));

    api.ea_stream_if = &stream_if;
    api.ea_stream_if_ctx = self.get();
    api.ea_packets_out = ea_packets_out;
    api.ea_packets_out_ctx = self.get();
    api.ea_get_ssl_ctx = ea_get_ssl_ctx;

    self->engine_ = lsquic_engine_new(flags, &api);
    if (self->engine_ == nullptr) {
      return LsQuicError::lsquic_engine_new;
    }

    io_context_ptr->post([weak_self{std::weak_ptr{self}}] {
      WEAK_LOCK(self);
      self->readLoop();
    });

    return self;
  }

  Engine::Engine(Private,
                 IoContextPtr io_context_ptr,
                 ConnectionIdCounter connection_id_counter,
                 TlsCertificate &&certificate,
                 Socket &&socket,
                 Socket::endpoint_type socket_local_endpoint,
                 std::weak_ptr<EngineController> controller)
      : MOVE_(io_context_ptr),
        MOVE_(connection_id_counter),
        MOVE_(certificate),
        MOVE_(socket),
        MOVE_(socket_local_endpoint),
        MOVE_(controller),
        timer_{*io_context_ptr_} {}

  Engine::~Engine() {
    if (engine_ != nullptr) {
      boost::asio::dispatch(*io_context_ptr_, [engine{engine_}] {
        // will call `Engine::on_conn_closed`, `Engine::on_close`.
        lsquic_engine_destroy(engine);
      });
    }
  }

  ConnectionPtrCoroOutcome Engine::connect(Self self, Address address) {
    SET_CORO_THREAD(self->io_context_ptr_);
    if (self->connecting_) {
      co_return ConnectionsError::ENGINE_CONNECT_ALREADY;
    }
    co_return co_await coroHandler<ConnectionPtrOutcome>(
        [&](CoroHandler<ConnectionPtrOutcome> &&handler) {
          self->connecting_.emplace(Connecting{
              .address = address,
              .handler = std::move(handler),
          });
          // will call `Engine::ea_get_ssl_ctx`, `Engine::on_new_conn`.
          lsquic_engine_connect(self->engine_,
                                N_LSQVER,
                                self->socket_local_endpoint_.data(),
                                make_endpoint(address).data(),
                                self.get(),
                                nullptr,
                                nullptr,
                                0,
                                nullptr,
                                0,
                                nullptr,
                                0);
          if (auto connecting = qtils::optionTake(self->connecting_)) {
            connecting->handler(LsQuicError::lsquic_engine_connect);
          }
          self->wantProcess();
        });
  }

  void Engine::wantFlush(StreamCtx *stream_ctx) {
    if (stream_ctx->want_flush) {
      return;
    }
    stream_ctx->want_flush = true;
    if (not stream_ctx->stream) {
      return;
    }
    want_flush_.emplace_back(stream_ctx->stream.value());
    wantProcess();
  }

  void Engine::wantProcess() {
    if (want_process_) {
      return;
    }
    want_process_ = true;
    boost::asio::post(*io_context_ptr_, [WEAK_SELF] {
      WEAK_LOCK(self);
      self->process();
    });
  }

  void Engine::process() {
    want_process_ = false;
    auto want_flush = std::exchange(want_flush_, {});
    for (auto &weak_stream : want_flush) {
      auto stream = weak_stream.lock();
      if (not stream) {
        continue;
      }
      if (not stream->stream_ctx_->ls_stream) {
        continue;
      }
      stream->stream_ctx_->want_flush = false;
      lsquic_stream_flush(stream->stream_ctx_->ls_stream.value());
    }
    // will call `Engine::on_new_conn`, `Engine::on_conn_closed`,
    // `Engine::on_new_stream`, `Engine::on_close`, `Engine::on_read`,
    // `Engine::on_write`, `Engine::ea_packets_out`.
    lsquic_engine_process_conns(engine_);
    int us = 0;
    if (not lsquic_engine_earliest_adv_tick(engine_, &us)) {
      return;
    }
    timer_.expires_after(std::chrono::microseconds{us});
    auto cb = [WEAK_SELF](boost::system::error_code ec) {
      WEAK_LOCK(self);
      if (ec) {
        return;
      }
      self->process();
    };
    timer_.async_wait(std::move(cb));
  }

  void Engine::readLoop() {
    // https://github.com/cbodley/nexus/blob/d1d8486f713fd089917331239d755932c7c8ed8e/src/socket.cc#L293
    while (true) {
      socklen_t len = socket_local_endpoint_.size();
      auto n = recvfrom(socket_.native_handle(),
                        reading_.buffer.data(),
                        reading_.buffer.size(),
                        0,
                        reading_.remote_endpoint.data(),
                        &len);
      if (n == -1) {
        if (errno == EAGAIN or errno == EWOULDBLOCK) {
          auto cb = [WEAK_SELF](boost::system::error_code ec) {
            WEAK_LOCK(self);
            if (ec) {
              return;
            }
            self->readLoop();
          };
          socket_.async_wait(boost::asio::socket_base::wait_read,
                             std::move(cb));
        }
        break;
      }
      // will call `Engine::on_hsk_done`, `Engine::ea_get_ssl_ctx`.
      lsquic_engine_packet_in(engine_,
                              reading_.buffer.data(),
                              n,
                              socket_local_endpoint_.data(),
                              reading_.remote_endpoint.data(),
                              this,
                              0);
    }
    process();
  }

  void Engine::destroyConnection(ConnCtx *conn_ctx) {
    conn_ctx->connection.reset();
    if (conn_ctx->ls_conn) {
      lsquic_conn_close(conn_ctx->ls_conn.value());
    } else {
      tryDelete(conn_ctx);
    }
  }

  StreamPtrCoroOutcome Engine::openStream(ConnCtx *conn_ctx,
                                          ProtocolId protocol_id) {
    if (not conn_ctx->ls_conn) {
      co_return ConnectionsError::CONNECTION_OPEN_CLOSED;
    }
    if (conn_ctx->open_stream) {
      co_return ConnectionsError::ENGINE_OPEN_STREAM_ALREADY;
    }
    if (lsquic_conn_n_avail_streams(conn_ctx->ls_conn.value()) == 0) {
      co_return ConnectionsError::ENGINE_OPEN_STREAM_TOO_MANY;
    }
    conn_ctx->open_stream = nullptr;
    // will call `Engine::on_new_stream`.
    lsquic_conn_make_stream(conn_ctx->ls_conn.value());
    auto stream = qtils::optionTake(conn_ctx->open_stream).value();
    if (stream == nullptr) {
      co_return LsQuicError::lsquic_conn_make_stream;
    }
    // stream not weak, because no other owners yet
    BOOST_OUTCOME_CO_TRY(co_await stream->writeProtocolId(protocol_id));
    co_return stream;
  }

  void Engine::destroyStream(StreamCtx *stream_ctx) {
    stream_ctx->stream.reset();
    if (stream_ctx->ls_stream) {
      lsquic_stream_close(stream_ctx->ls_stream.value());
    } else {
      tryDelete(stream_ctx);
    }
  }

  void Engine::streamAccept(StreamPtr &&stream) {
    coroSpawn(*io_context_ptr_,
              [weak_controller{controller_},
               MOVE(stream)]() mutable -> CoroOutcome<void> {
                // stream not weak, because no other owners yet
                BOOST_OUTCOME_CO_TRY(auto protocol_id,
                                     co_await stream->readProtocolId());
                if (auto controller = weak_controller.lock()) {
                  auto &connection = stream->connection_;
                  controller->onStreamAccept(
                      connection, protocol_id, std::move(stream));
                }
                co_return outcome::success();
              });
  }

  void Engine::streamReadFin(StreamCtx *stream_ctx) {
    if (not stream_ctx->ls_stream) {
      return;
    }
    lsquic_stream_shutdown(stream_ctx->ls_stream.value(), SHUT_RD);
  }

  void Engine::streamWriteFin(StreamCtx *stream_ctx) {
    if (not stream_ctx->ls_stream) {
      return;
    }
    lsquic_stream_shutdown(stream_ctx->ls_stream.value(), SHUT_WR);
  }

  CoroOutcome<bool> Engine::streamReadRaw(StreamCtx *stream_ctx,
                                          qtils::BytesOut message) {
    if (stream_ctx->reading) {
      throw std::logic_error{"Engine::streamReadRaw duplicate"};
    }
    auto remaining = message;
    while (not remaining.empty()) {
      if (not stream_ctx->ls_stream) {
        co_return ConnectionsError::STREAM_READ_CLOSED;
      }
      auto n = lsquic_stream_read(
          stream_ctx->ls_stream.value(), remaining.data(), remaining.size());
      if (n == 0) {
        if (remaining.size() == message.size()) {
          co_return false;
        } else {
          co_return ConnectionsError::STREAM_READ_CLOSED;
        }
      }
      if (n == -1) {
        if (errno != EWOULDBLOCK) {
          co_return ConnectionsError::STREAM_READ_CLOSED;
        }
        co_await coroHandler<void>([&](CoroHandler<void> &&handler) {
          stream_ctx->reading.emplace(std::move(handler));
          lsquic_stream_wantread(stream_ctx->ls_stream.value(), 1);
        });
        continue;
      }
      remaining = remaining.subspan(n);
    }
    co_return true;
  }

  CoroOutcome<void> Engine::streamWriteRaw(StreamCtx *stream_ctx,
                                           qtils::BytesIn message) {
    if (stream_ctx->writing) {
      throw std::logic_error{"Engine::streamWriteRaw duplicate"};
    }
    auto remaining = message;
    while (not remaining.empty()) {
      if (not stream_ctx->ls_stream) {
        co_return ConnectionsError::STREAM_WRITE_CLOSED;
      }
      auto n = lsquic_stream_write(
          stream_ctx->ls_stream.value(), remaining.data(), remaining.size());
      if (n < 0) {
        co_return ConnectionsError::STREAM_WRITE_CLOSED;
      }
      if (n != 0) {
        remaining = remaining.subspan(n);
        auto self = stream_ctx->engine.lock();
        if (not self) {
          co_return ConnectionsError::STREAM_WRITE_CLOSED;
        }
        self->wantFlush(stream_ctx);
      }
      if (remaining.empty()) {
        break;
      }
      co_await coroHandler<void>([&](CoroHandler<void> &&handler) {
        stream_ctx->writing.emplace(std::move(handler));
        lsquic_stream_wantwrite(stream_ctx->ls_stream.value(), 1);
      });
    }
    co_return outcome::success();
  }

  lsquic_conn_ctx_t *Engine::on_new_conn(void *void_self,
                                         lsquic_conn_t *ls_conn) {
    SELF_FROM_VOID;
    auto connecting = qtils::optionTake(self->connecting_);
    auto is_connecting = connecting.has_value();
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto *conn_ctx = new ConnCtx{
        .engine = self->weak_from_this(),
        .ls_conn = ls_conn,
        .connecting = std::move(connecting),
    };
    auto *ls_conn_ctx = to_ls(conn_ctx);
    lsquic_conn_set_ctx(ls_conn, ls_conn_ctx);
    if (not is_connecting) {
      // lsquic doesn't call `on_hsk_done` for incoming connection
      on_hsk_done(ls_conn, LSQ_HSK_OK);
    }
    return ls_conn_ctx;
  }

  void Engine::on_conn_closed(lsquic_conn_t *ls_conn) {
    auto *conn_ctx = from_ls<ConnCtx>(lsquic_conn_get_ctx(ls_conn));
    conn_ctx->ls_conn.reset();
    lsquic_conn_set_ctx(ls_conn, nullptr);
    if (auto connecting = qtils::optionTake(conn_ctx->connecting)) {
      connecting->handler(ConnectionsError::ENGINE_CONNECT_CLOSED);
    } else if (auto self = conn_ctx->engine.lock()) {
      if (auto controller = self->controller_.lock()) {
        controller->onConnectionClose(conn_ctx->info.value());
      }
    }
    tryDelete(conn_ctx);
  }

  void Engine::on_hsk_done(lsquic_conn_t *ls_conn, lsquic_hsk_status status) {
    auto *conn_ctx = from_ls<ConnCtx>(lsquic_conn_get_ctx(ls_conn));
    auto self = conn_ctx->engine.lock();
    if (not self) {
      return;
    }
    auto ok = status == LSQ_HSK_OK or status == LSQ_HSK_RESUMED_OK;
    auto connecting = qtils::optionTake(conn_ctx->connecting);
    auto connection_result = [&]() -> ConnectionPtrOutcome {
      if (not ok) {
        return ConnectionsError::HANDSHAKE_FAILED;
      }
      OUTCOME_TRY(key, TlsCertificate::get_key(lsquic_conn_ssl(ls_conn)));
      if (connecting and key != connecting->address.key) {
        return ConnectionsError::ENGINE_CONNECT_KEY_MISMATCH;
      }
      conn_ctx->info = ConnectionInfo{
          .id = self->connection_id_counter_.make(),
          .key = key,
      };
      auto connection = std::make_shared<Connection>(
          self->io_context_ptr_, conn_ctx, conn_ctx->info.value());
      conn_ctx->connection = connection;
      return connection;
    }();
    if (not connection_result) {
      lsquic_conn_close(ls_conn);
    }
    if (connecting) {
      connecting->handler(std::move(connection_result));
    } else if (connection_result) {
      auto &connection = connection_result.value();
      if (auto controller = self->controller_.lock()) {
        controller->onConnectionAccept(std::move(connection));
      }
    }
  }

  lsquic_stream_ctx_t *Engine::on_new_stream(void *void_self,
                                             lsquic_stream_t *ls_stream) {
    SELF_FROM_VOID;
    auto *conn_ctx =
        from_ls<ConnCtx>(lsquic_conn_get_ctx(lsquic_stream_conn(ls_stream)));
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto *stream_ctx = new StreamCtx{
        .engine = self->weak_from_this(),
        .ls_stream = ls_stream,
    };
    ConnectionPtr connection;
    if (conn_ctx->connection) {
      connection = conn_ctx->connection->lock();
    }
    if (connection) {
      auto stream = std::make_shared<Stream>(
          self->io_context_ptr_, connection, stream_ctx);
      stream_ctx->stream = stream;
      if (conn_ctx->open_stream) {
        conn_ctx->open_stream.value() = stream;
      } else {
        self->streamAccept(std::move(stream));
      }
    } else {
      lsquic_stream_close(ls_stream);
    }
    return to_ls(stream_ctx);
  }

  void Engine::on_close(lsquic_stream_t *ls_stream,
                        lsquic_stream_ctx_t *ls_stream_ctx) {
    auto *stream_ctx = from_ls<StreamCtx>(ls_stream_ctx);
    stream_ctx->ls_stream.reset();
    if (auto reading = qtils::optionTake(stream_ctx->reading)) {
      reading.value()();
    }
    if (auto writing = qtils::optionTake(stream_ctx->writing)) {
      writing.value()();
    }
    tryDelete(stream_ctx);
  }

  void Engine::on_read(lsquic_stream_t *ls_stream,
                       lsquic_stream_ctx_t *ls_stream_ctx) {
    lsquic_stream_wantread(ls_stream, 0);
    auto *stream_ctx = from_ls<StreamCtx>(ls_stream_ctx);
    if (auto reading = qtils::optionTake(stream_ctx->reading)) {
      reading.value()();
    }
  }

  void Engine::on_write(lsquic_stream_t *ls_stream,
                        lsquic_stream_ctx_t *ls_stream_ctx) {
    lsquic_stream_wantwrite(ls_stream, 0);
    auto *stream_ctx = from_ls<StreamCtx>(ls_stream_ctx);
    if (auto writing = qtils::optionTake(stream_ctx->writing)) {
      writing.value()();
    }
  }

  ssl_ctx_st *Engine::ea_get_ssl_ctx(void *void_self, const sockaddr *) {
    SELF_FROM_VOID;
    return self->certificate_;
  }

  int Engine::ea_packets_out(void *void_self,
                             const lsquic_out_spec *out_spec,
                             unsigned n_packets_out) {
    SELF_FROM_VOID;
    // https://github.com/cbodley/nexus/blob/d1d8486f713fd089917331239d755932c7c8ed8e/src/socket.cc#L218
    int r = 0;
    for (auto &spec : std::span{out_spec, n_packets_out}) {
      msghdr msg{};
      msg.msg_iov = spec.iov;
      msg.msg_iovlen = spec.iovlen;
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
      msg.msg_name = const_cast<sockaddr *>(spec.dest_sa);
      msg.msg_namelen = spec.dest_sa->sa_family == AF_INET
                          ? sizeof(sockaddr_in)
                          : sizeof(sockaddr_in6);
      auto n = sendmsg(self->socket_.native_handle(), &msg, 0);
      if (n == -1) {
        if (errno == EAGAIN or errno == EWOULDBLOCK) {
          auto cb = [weak_self{self->weak_from_this()}](
                        boost::system::error_code ec) {
            WEAK_LOCK(self);
            if (ec) {
              return;
            }
            // will call `Engine::ea_packets_out`.
            lsquic_engine_send_unsent_packets(self->engine_);
          };
          self->socket_.async_wait(Socket::wait_write, std::move(cb));
        }
        break;
      }
      ++r;
    }
    return r;
  }
}  // namespace jam::snp::lsquic
