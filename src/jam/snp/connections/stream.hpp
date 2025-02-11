/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam/coro/coro.hpp>
#include <jam/coro/io_context_ptr.hpp>
#include <jam/snp/connections/connection_ptr.hpp>
#include <jam/snp/connections/message_size.hpp>
#include <jam/snp/connections/protocol_id.hpp>
#include <qtils/bytes.hpp>

namespace jam::snp::lsquic {
  struct StreamCtx;
  class Engine;
}  // namespace jam::snp::lsquic

namespace jam::snp {
  class Stream {
    friend lsquic::Engine;

   public:
    using Self = std::shared_ptr<Stream>;

    Stream(IoContextPtr io_context_ptr,
           ConnectionPtr connection,
           lsquic::StreamCtx *stream_ctx);
    /**
     * Will close stream and decrement `Connection` shared use count.
     */
    ~Stream();

    // https://github.com/zdave-parity/jam-np/blob/5d374b53578cdd93646e3ee19e2b19ea132317b8/simple.md?plain=1#L109-L111
    /**
     * Read whole size prefixed message, no more than `max` bytes.
     * Returns `true` if message was read, or `false` if fin was received or
     * stream was closed.
     */
    static CoroOutcome<bool> read(Self self,
                                  qtils::Bytes &buffer,
                                  MessageSize max);

    /**
     * Close reading side of stream.
     */
    static Coro<void> readFin(Self self);

    // https://github.com/zdave-parity/jam-np/blob/5d374b53578cdd93646e3ee19e2b19ea132317b8/simple.md?plain=1#L109-L111
    /**
     * Write while size prefixed message.
     */
    static CoroOutcome<void> write(Self self, qtils::BytesIn message);

    /**
     * Write fin.
     * Closes writing side of stream.
     */
    static Coro<void> writeFin(Self self);

   private:
    /**
     * Read protocol id (server).
     */
    CoroOutcome<ProtocolId> readProtocolId();
    /**
     * Write protocol id (client).
     */
    CoroOutcome<void> writeProtocolId(ProtocolId protocol_id);

    /**
     * `Stream`, `Engine` operations executed on one `IoContextPtr` thread.
     */
    IoContextPtr io_context_ptr_;
    /**
     * `Stream` keeps `Connection` shared use count alive.
     */
    ConnectionPtr connection_;
    lsquic::StreamCtx *stream_ctx_;
  };
}  // namespace jam::snp
