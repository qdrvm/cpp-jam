/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "coro/coro.hpp"
#include "coro/io_context_ptr.hpp"
#include "snp/connections/connection_info.hpp"
#include "snp/connections/protocol_id.hpp"
#include "snp/connections/stream_ptr.hpp"

namespace jam::snp::lsquic {
  struct ConnCtx;
  class Engine;
}  // namespace jam::snp::lsquic

namespace jam::snp {
  class Connection {
    friend lsquic::Engine;

   public:
    using Self = std::shared_ptr<Connection>;

    Connection(IoContextPtr io_context_ptr,
               lsquic::ConnCtx *conn_ctx,
               ConnectionInfo info);
    ~Connection();

    const ConnectionInfo &info() const;

    /**
     * Open stream with specified `ProtocolId`.
     */
    static StreamPtrCoroOutcome open(Self self, ProtocolId protocol_id);

   private:
    IoContextPtr io_context_ptr_;
    lsquic::ConnCtx *conn_ctx_;
    ConnectionInfo info_;
  };
}  // namespace jam::snp
