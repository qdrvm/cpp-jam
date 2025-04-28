/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "modules/networking/networking.hpp"

#include <boost/outcome/try.hpp>
#include <qtils/to_shared_ptr.hpp>

#include "coro/spawn.hpp"
#include "coro/weak.hpp"
#include "scale/jam_scale.hpp"
#include "snp/connections/address.hpp"
#include "snp/connections/connection.hpp"
#include "snp/connections/connections.hpp"
#include "snp/connections/stream.hpp"
#include "tests/utils/ed25519_literal.hpp"

namespace jam::modules {
  // TODO: remove debug code
  const auto debug_server_key =
      "f8dfdb0f1103d9fb2905204ac32529d5f148761c4321b2865b0a40e15be75f57"_ed25519;
  const snp::Address debug_server_address{
      snp::Address::kLocal,
      10000,
      crypto::ed25519::get_public(debug_server_key),
  };
  inline auto debugIsServer() {
    auto env = getenv("SERVER");
    return env and std::string_view{env} == "1";
  }

  // https://github.com/zdave-parity/jam-np/blob/main/simple.md#up-0-block-announcement
  const auto kBlockAnnouncementProtocolId =
      snp::ProtocolId::make(0, true).value();

  // https://github.com/zdave-parity/jam-np/blob/main/simple.md#ce-128-block-request
  const auto kBlockRequestProtocolId =
      snp::ProtocolId::make(128, false).value();

  NetworkingImpl::NetworkingImpl(
      qtils::StrictSharedPtr<NetworkingLoader> loader,
      qtils::StrictSharedPtr<log::LoggingSystem> logging_system)
      : loader_(loader),
        logger_(logging_system->getLogger("Networking", "networking_module")),
        io_context_ptr_{std::make_shared<boost::asio::io_context>()},
        io_thread_{[io_context_ptr{io_context_ptr_}] {
          auto work_guard = boost::asio::make_work_guard(*io_context_ptr);
          // TODO: watchdog
          io_context_ptr->run();
        }},
        connections_{std::make_shared<snp::Connections>(
            io_context_ptr_, logging_system, [] {
              // TODO: config
              snp::ConnectionsConfig config;
              config.genesis = {};
              if (debugIsServer()) {
                config.keypair = debug_server_key;
                config.listen_port = debug_server_address.port;
              } else {
                crypto::ed25519::Seed seed;
                for (auto &x : seed) {
                  x = random();
                }
                config.keypair = crypto::ed25519::from_seed(seed);
              }
              return config;
            }())} {}

  NetworkingImpl::~NetworkingImpl() {
    io_context_ptr_->stop();
    io_thread_.join();
  }

  void NetworkingImpl::on_loaded_success() {
    SL_INFO(logger_, "Loaded success");
    coroSpawn(
        *io_context_ptr_, [self{shared_from_this()}]() mutable -> Coro<void> {
          auto init_result =
              co_await self->connections_->init(self->connections_, self);
          if (not init_result.has_value()) {
            SL_ERROR(self->logger_,
                     "Connections::init error: {}",
                     init_result.error());
          }

          co_await self->connections_->serve(
              self->connections_,
              kBlockAnnouncementProtocolId,
              [weak_self{std::weak_ptr{self}}](
                  snp::ConnectionInfo info,
                  snp::StreamPtr stream) mutable -> CoroOutcome<void> {
                auto self = weak_self.lock();
                if (not self) {
                  co_return NetworkingError::NETWORKING_DESTROYED;
                }
                co_await self->handleBlockAnnouncement(self, info.key, stream);
                co_return outcome::success();
              });

          co_await self->connections_->serve(
              self->connections_,
              kBlockRequestProtocolId,
              [weak_self{std::weak_ptr{self}}](
                  snp::ConnectionInfo info,
                  snp::StreamPtr stream) mutable -> CoroOutcome<void> {
                auto self = weak_self.lock();
                if (not self) {
                  co_return NetworkingError::NETWORKING_DESTROYED;
                }
                qtils::Bytes buffer;
                BOOST_OUTCOME_CO_TRY(
                    auto request,
                    co_await read<BlockRequest>(self, stream, buffer));
                // TODO: get response
                BlockResponse response;
                BOOST_OUTCOME_CO_TRY(
                    co_await write(self, stream, buffer, response));
                co_return outcome::success();
              });

          if (not debugIsServer()) {
            SL_INFO(self->logger_, "DEBUG: client, connect to server");
            auto connection_result =
                CORO_WEAK_AWAIT(self,
                                self->connections_->connect(
                                    self->connections_, debug_server_address));
            if (not connection_result.has_value()) {
              SL_ERROR(self->logger_,
                       "Connections::connect error: {}",
                       connection_result.error());
            }
          }
        });
  }

  void NetworkingImpl::on_loading_is_finished() {
    SL_INFO(logger_, "Loading is finished");
  }

  void NetworkingImpl::on_block_request(
      std::shared_ptr<const messages::SendBlockRequest> msg) {
    SL_INFO(logger_, "Block requested");
    coroSpawn(*io_context_ptr_,
              [self{shared_from_this()}, msg]() mutable -> Coro<void> {
                auto result = co_await self->sendRequest<BlockResponse>(
                    self, msg->to_peer, kBlockRequestProtocolId, msg->request);
                self->loader_->dispatch_block_response(
                    qtils::toSharedPtr(messages::BlockResponseReceived{
                        .for_ctx = msg->ctx,
                        .response_result = std::move(result),
                    }));
              });
  }

  void NetworkingImpl::onOpen(snp::Key key) {
    loader_->dispatch_peer_connected(qtils::toSharedPtr(
        messages::PeerConnectedMessage{.peer = {.key = key}}));
  }

  void NetworkingImpl::onClose(snp::Key key) {
    loader_->dispatch_peer_disconnected(qtils::toSharedPtr(
        messages::PeerDisconnectedMessage{.peer = {.key = key}}));
  }

  void NetworkingImpl::onConnectionChange(snp::ConnectionPtr connection) {
    auto connection_info = connection->info();
    if (connection_info.outbound) {
      coroSpawn(
          *io_context_ptr_,
          [self{shared_from_this()},
           connection,
           key{connection_info.key}]() mutable -> Coro<void> {
            auto stream_result = CORO_WEAK_AWAIT(
                self,
                connection->open(connection, kBlockAnnouncementProtocolId));
            if (not stream_result.has_value()) {
              SL_INFO(self->logger_,
                      "can't open block announcement stream: {}",
                      stream_result.error());
              co_return;
            }
            auto &stream = stream_result.value();
            connection.reset();
            co_await self->handleBlockAnnouncement(self, key, stream);
          });
    }
  }

  template <typename T>
  CoroOutcome<T> NetworkingImpl::read(SelfSPtr &self,
                                      snp::StreamPtr stream,
                                      qtils::Bytes buffer) {
    BOOST_OUTCOME_CO_TRY(
        auto read,
        CORO_WEAK_AWAIT(self,
                        stream->read(stream, buffer, snp::kMessageSizeMax),
                        NetworkingError::NETWORKING_DESTROYED));
    if (not read) {
      co_return NetworkingError::END_OF_STREAM;
    }
    co_return decode_with_config<T>(buffer);
  }
  template <typename T>
  CoroOutcome<void> NetworkingImpl::write(SelfSPtr &self,
                                          snp::StreamPtr stream,
                                          qtils::Bytes buffer,
                                          const T &message) {
    BOOST_OUTCOME_CO_TRY(buffer, encode_with_config(message));
    BOOST_OUTCOME_CO_TRY(
        CORO_WEAK_AWAIT(self,
                        stream->write(stream, buffer),
                        NetworkingError::NETWORKING_DESTROYED));
    co_return outcome::success();
  }

  template <typename Response, typename Request>
  CoroOutcome<Response> NetworkingImpl::sendRequest(SelfSPtr &self,
                                                    PeerId peer_id,
                                                    snp::ProtocolId protocol,
                                                    const Request &request) {
    BOOST_OUTCOME_CO_TRY(auto connection,
                         co_await self->connections_->getConnected(
                             self->connections_, peer_id.key));
    BOOST_OUTCOME_CO_TRY(
        auto stream,
        CORO_WEAK_AWAIT(self,
                        connection->open(connection, protocol),
                        NetworkingError::NETWORKING_DESTROYED));
    connection.reset();
    qtils::Bytes buffer;
    BOOST_OUTCOME_CO_TRY(co_await write(self, stream, buffer, request));
    co_await stream->shutdownWrite(stream);
    BOOST_OUTCOME_CO_TRY(auto response,
                         co_await read<Response>(self, stream, buffer));
    co_await stream->shutdownRead(stream);
    co_return response;
  }

  Coro<void> NetworkingImpl::handleBlockAnnouncement(SelfSPtr &self,
                                                     snp::Key key,
                                                     snp::StreamPtr stream) {
    PeerId peer_id{.key = key};
    co_await coroSpawn([self, stream]() mutable -> Coro<void> {
      // TODO: get handshake
      BlockAnnouncementHandshake handshake;
      qtils::Bytes buffer;
      auto write_result = co_await self->write(self, stream, buffer, handshake);
      if (not write_result.has_value()) {
        co_await stream->shutdown(stream);
        co_return;
      }
      // TODO: save writer
    });
    qtils::Bytes buffer;
    auto handshake_result =
        co_await self->read<BlockAnnouncementHandshake>(self, stream, buffer);
    if (handshake_result.has_value()) {
      auto &handshake = handshake_result.value();
      self->loader_->dispatch_block_announcement_handshake(
          qtils::toSharedPtr(messages::BlockAnnouncementHandshakeReceived{
              .from_peer = peer_id,
              .notification = handshake,
          }));
      while (true) {
        auto message_result =
            co_await self->read<BlockAnnouncement>(self, stream, buffer);
        if (not message_result.has_value()) {
          break;
        }
        auto &message = message_result.value();
        self->loader_->dispatch_block_announce(
            qtils::toSharedPtr(messages::BlockAnnouncementReceived{
                .from_peer = peer_id,
                .notification = message,
            }));
      }
    }
    co_await stream->shutdown(stream);
  }
}  // namespace jam::modules
