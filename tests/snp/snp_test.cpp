/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <boost/outcome/try.hpp>
#include <qtils/test/outcome.hpp>

#include "crypto/ed25519_literal.hpp"
#include "log/simple.hpp"
#include "snp/connections/address.hpp"
#include "snp/connections/connection.hpp"
#include "snp/connections/connections.hpp"
#include "snp/connections/controller.hpp"
#include "snp/connections/stream.hpp"

// bug: gtest "ASSERT_*" contains "return"
#define CO_ASSERT_OUTCOME_SUCCESS(result, expression)   \
  auto result = ({                                      \
    auto _co_assert_outcome_success = (expression);     \
    EXPECT_OUTCOME_SUCCESS(_co_assert_outcome_success); \
    _co_assert_outcome_success.value();                 \
  })

using jam::Coro;
using jam::CoroHandler;
using jam::coroHandler;
using jam::CoroOutcome;
using jam::coroSpawn;
using jam::GenesisHash;
using jam::snp::Address;
using jam::snp::ConnectionInfo;
using jam::snp::Connections;
using jam::snp::ConnectionsConfig;
using jam::snp::ConnectionsController;
using jam::snp::Key;
using jam::snp::ProtocolId;
using jam::snp::StreamPtr;

auto logsys = jam::log::simpleLoggingSystem();

GenesisHash genesis;
auto server_key =
    "f8dfdb0f1103d9fb2905204ac32529d5f148761c4321b2865b0a40e15be75f57"_ed25519;
Address server_address{
    Address::kLocal,
    10000,
    jam::crypto::ed25519::get_public(server_key),
};
ProtocolId protocol_id = ProtocolId::make(128, false).value();

/**
 * Client connects to server, opens stream, writes message, and receives
 * response.
 */
TEST(SnpTest, Snp) {
  enum class State : uint8_t {
    Start,
    Connecting,
    OpeningStream,
    WritingRequest,
    WritingResponse,
    Done,
  };
  auto state = State::Start;

  auto io_context_ptr = std::make_shared<boost::asio::io_context>();
  // server
  coroSpawn(*io_context_ptr, [io_context_ptr, &state]() -> Coro<void> {
    ConnectionsConfig config{
        .genesis = genesis,
        .keypair = server_key,
        .listen_port = server_address.port,
    };
    auto connections =
        std::make_shared<Connections>(io_context_ptr, logsys, config);
    EXPECT_OUTCOME_SUCCESS(co_await connections->init(connections, {}));

    co_await connections->serve(
        connections,
        protocol_id,
        [&state](ConnectionInfo info, StreamPtr stream) -> CoroOutcome<void> {
          qtils::Bytes message;
          EXPECT_OUTCOME_SUCCESS(read,
                                 co_await stream->read(stream, message, 1));
          co_await stream->shutdownRead(stream);
          EXPECT_TRUE(read);
          EXPECT_EQ(message.size(), 1);
          message[0] += 1;

          state = State::WritingResponse;
          EXPECT_OUTCOME_SUCCESS(co_await stream->write(stream, message));
          co_await stream->shutdownWrite(stream);
          co_return outcome::success();
        });

    std::optional<CoroHandler<void>> work_guard;
    co_await coroHandler<void>([&](CoroHandler<void> &&handler) {
      work_guard.emplace(std::move(handler));
    });
  });
  // client
  coroSpawn(*io_context_ptr, [io_context_ptr, &state]() -> Coro<void> {
    ConnectionsConfig config{
        .genesis = genesis,
        .keypair =
            "96c891b8726cb18c781aefc082dbafcb827e16c8f18f22d461e83eabd618e780"_ed25519,
    };
    auto connections =
        std::make_shared<Connections>(io_context_ptr, logsys, config);
    EXPECT_OUTCOME_SUCCESS(co_await connections->init(connections, {}));

    state = State::Connecting;
    CO_ASSERT_OUTCOME_SUCCESS(
        connection, co_await connections->connect(connections, server_address));

    state = State::OpeningStream;
    CO_ASSERT_OUTCOME_SUCCESS(
        stream, co_await connection->open(connection, protocol_id));

    state = State::WritingRequest;
    qtils::Bytes message{1};
    EXPECT_OUTCOME_SUCCESS(co_await stream->write(stream, message));
    co_await stream->shutdownWrite(stream);

    EXPECT_OUTCOME_SUCCESS(read, co_await stream->read(stream, message, 1));
    co_await stream->shutdownRead(stream);
    EXPECT_TRUE(read);
    EXPECT_EQ(message.size(), 1);
    EXPECT_EQ(message[0], 2);

    state = State::Done;
    io_context_ptr->stop();
  });
  io_context_ptr->run();
  EXPECT_EQ(state, State::Done);
}
