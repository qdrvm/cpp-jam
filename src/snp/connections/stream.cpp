/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "snp/connections/stream.hpp"

#include <boost/asio/dispatch.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/outcome/try.hpp>

#include "coro/set_thread.hpp"
#include "coro/weak.hpp"
#include "snp/connections/error.hpp"
#include "snp/connections/lsquic/engine.hpp"

namespace jam::snp {
  using lsquic::Engine;

  using ProtocolIdBytes = qtils::BytesN<1>;
  using MessageSizeBytes = qtils::BytesN<sizeof(MessageSize)>;

  Stream::Stream(IoContextPtr io_context_ptr,
                 ConnectionPtr connection,
                 lsquic::StreamCtx *stream_ctx)
      : io_context_ptr_{std::move(io_context_ptr)},
        connection_{std::move(connection)},
        stream_ctx_{std::move(stream_ctx)} {}

  Stream::~Stream() {
    boost::asio::dispatch(*io_context_ptr_, [stream_ctx{stream_ctx_}] {
      Engine::destroyStream(stream_ctx);
    });
  }

  CoroOutcome<bool> Stream::read(Self self,
                                 qtils::Bytes &buffer,
                                 MessageSize max) {
    co_await setCoroThread(self->io_context_ptr_);
    MessageSizeBytes size_bytes;
    BOOST_OUTCOME_CO_TRY(
        auto read_size,
        CORO_WEAK_AWAIT(self,
                        Engine::streamReadRaw(self->stream_ctx_, size_bytes),
                        ConnectionsError::STREAM_READ_DESTROYED));
    if (not read_size) {
      co_return false;
    }
    auto size = boost::endian::load_little_u32(size_bytes.data());
    if (size > max) {
      co_return ConnectionsError::STREAM_READ_TOO_BIG;
    }
    buffer.resize(size);
    BOOST_OUTCOME_CO_TRY(
        auto read_message,
        CORO_WEAK_AWAIT(self,
                        Engine::streamReadRaw(self->stream_ctx_, buffer),
                        ConnectionsError::STREAM_READ_DESTROYED));
    if (not read_message) {
      co_return ConnectionsError::STREAM_READ_CLOSED;
    }
    co_return true;
  }

  Coro<void> Stream::readFin(Self self) {
    co_await setCoroThread(self->io_context_ptr_);
    Engine::streamReadFin(self->stream_ctx_);
    co_return;
  }

  CoroOutcome<void> Stream::write(Self self, qtils::BytesIn message) {
    co_await setCoroThread(self->io_context_ptr_);
    MessageSizeBytes size_bytes;
    auto size = message.size();
    if (size > kMessageSizeMax) {
      throw std::logic_error{"Stream::write max"};
    }
    boost::endian::store_little_u32(size_bytes.data(), size);
    BOOST_OUTCOME_CO_TRY(
        CORO_WEAK_AWAIT(self,
                        Engine::streamWriteRaw(self->stream_ctx_, size_bytes),
                        ConnectionsError::STREAM_WRITE_DESTROYED));
    BOOST_OUTCOME_CO_TRY(
        CORO_WEAK_AWAIT(self,
                        Engine::streamWriteRaw(self->stream_ctx_, message),
                        ConnectionsError::STREAM_WRITE_DESTROYED));
    co_return outcome::success();
  }

  Coro<void> Stream::writeFin(Self self) {
    co_await setCoroThread(self->io_context_ptr_);
    Engine::streamWriteFin(self->stream_ctx_);
    co_return;
  }

  CoroOutcome<ProtocolId> Stream::readProtocolId() {
    ProtocolIdBytes bytes;
    BOOST_OUTCOME_CO_TRY(auto read,
                         co_await Engine::streamReadRaw(stream_ctx_, bytes));
    if (not read) {
      co_return ConnectionsError::STREAM_READ_PROTOCOL_ID_CLOSED;
    }
    co_return ProtocolId{bytes[0]};
  }

  CoroOutcome<void> Stream::writeProtocolId(ProtocolId protocol_id) {
    ProtocolIdBytes bytes{protocol_id.id()};
    co_return co_await Engine::streamWriteRaw(stream_ctx_, bytes);
  }
}  // namespace jam::snp
