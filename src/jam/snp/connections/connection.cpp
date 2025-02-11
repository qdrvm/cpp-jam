/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <jam/snp/connections/connection.hpp>

#include <TODO_qtils/macro/move.hpp>
#include <boost/asio/dispatch.hpp>
#include <jam/coro/set_thread.hpp>
#include <jam/snp/connections/error.hpp>
#include <jam/snp/connections/lsquic/engine.hpp>

namespace jam::snp {
  using lsquic::Engine;

  Connection::Connection(IoContextPtr io_context_ptr,
                         lsquic::ConnCtx *conn_ctx,
                         ConnectionInfo info)
      : MOVE_(io_context_ptr), MOVE_(conn_ctx), MOVE_(info) {}

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
