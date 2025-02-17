/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "snp/connections/connection.hpp"

#include <boost/asio/dispatch.hpp>

#include "coro/set_thread.hpp"
#include "snp/connections/error.hpp"
#include "snp/connections/lsquic/engine.hpp"

namespace jam::snp {
  using lsquic::Engine;

  Connection::Connection(IoContextPtr io_context_ptr,
                         lsquic::ConnCtx *conn_ctx,
                         ConnectionInfo info)
      : io_context_ptr_{std::move(io_context_ptr)},
        conn_ctx_{std::move(conn_ctx)},
        info_{std::move(info)} {}

  Connection::~Connection() {
    boost::asio::dispatch(*io_context_ptr_, [conn_ctx{conn_ctx_}] {
      Engine::destroyConnection(conn_ctx);
    });
  }

  const ConnectionInfo &Connection::info() const {
    return info_;
  }

  StreamPtrCoroOutcome Connection::open(Self self, ProtocolId protocol_id) {
    SET_CORO_THREAD(self->io_context_ptr_);
    co_return co_await Engine::openStream(self->conn_ctx_, protocol_id);
  }
}  // namespace jam::snp
