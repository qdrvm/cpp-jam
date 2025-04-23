/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "snp/connections/connections.hpp"

#include <TODO_qtils/map_entry.hpp>
#include <TODO_qtils/variant_get.hpp>
#include <boost/outcome/try.hpp>

#include "coro/set_thread.hpp"
#include "coro/spawn.hpp"
#include "coro/weak.hpp"
#include "coro/yield.hpp"
#include "snp/connections/address.hpp"
#include "snp/connections/connection.hpp"
#include "snp/connections/controller.hpp"
#include "snp/connections/error.hpp"
#include "snp/connections/lsquic/engine.hpp"

namespace jam::snp {
  inline void todoPreferConnection() {
    // TODO(turuslan): how to deduplicate connections between two peers?
    throw std::logic_error{"TODO: prefer connection"};
  }

  Connections::Connections(IoContextPtr io_context_ptr,
                           std::shared_ptr<log::LoggingSystem> logsys,
                           ConnectionsConfig config)
      : io_context_ptr_{std::move(io_context_ptr)},
        logsys_{std::move(logsys)},
        init_{io_context_ptr_},
        config_{std::move(config)},
        key_{crypto::ed25519::get_public(config_.keypair)} {}

  CoroOutcome<void> Connections::init(
      SelfSPtr self, std::weak_ptr<ConnectionsController> controller) {
    co_await setCoroThread(self->io_context_ptr_);
    auto init = self->init_.init();
    self->controller_ = std::move(controller);
    BOOST_OUTCOME_CO_TRY(auto certificate, TlsCertificate::make(self->config_));
    BOOST_OUTCOME_CO_TRY(self->client_,
                         lsquic::Engine::make(self->io_context_ptr_,
                                              self->logsys_,
                                              certificate,
                                              std::nullopt,
                                              self));
    if (self->config_.listen_port.has_value()) {
      BOOST_OUTCOME_CO_TRY(self->server_,
                           lsquic::Engine::make(self->io_context_ptr_,
                                                self->logsys_,
                                                certificate,
                                                self->config_.listen_port,
                                                self));
    }
    init.ready();
    co_return outcome::success();
  }

  const Key &Connections::key() const {
    return key_;
  }

  ConnectionPtrCoroOutcome Connections::connect(SelfSPtr self,
                                                Address address) {
    co_await setCoroThread(self->io_context_ptr_);
    if (not co_await self->init_.ready()) {
      co_return ConnectionsError::CONNECTIONS_INIT;
    }
    auto state = qtils::entry(self->connections_, address.key);
    if (not state) {
      state.insert(
          std::make_shared<Connecting::element_type>(self->io_context_ptr_));
      co_await coroSpawn([self, address, state]() mutable -> Coro<void> {
        co_await coroYield();
        auto connection_result = CORO_WEAK_AWAIT(
            self, self->client_->connect(self->client_, address));
        auto state = qtils::entry(self->connections_, address.key);
        if (not state or not std::holds_alternative<Connecting>(*state)) {
          todoPreferConnection();
        }
        auto connecting = std::move(std::get<Connecting>(*state));
        if (connection_result) {
          auto &connection = connection_result.value();
          *state = Connected{connection};
          if (auto controller = self->controller_.lock()) {
            controller->onOpen(address.key);
          }
        } else {
          state.erase();
        }
        CORO_WEAK_AWAIT_V(
            self, connecting->set(connecting, std::move(connection_result)));
      });
    } else if (auto connected = qtils::variantGet<Connected>(*state)) {
      co_return *connected;
    }
    auto connecting = std::get<Connecting>(*state);
    self.reset();
    co_return co_await connecting->get(connecting);
  }

  Coro<void> Connections::serve(SelfSPtr self,
                                ProtocolId protocol_id,
                                ServeProtocol serve) {
    co_await setCoroThread(self->io_context_ptr_);
    qtils::entry(self->protocols_, protocol_id).insert(std::move(serve));
  }

  void Connections::onConnectionAccept(ConnectionPtr connection) {
    auto state = entry(connections_, connection->info().key);
    if (state) {
      todoPreferConnection();
    }
    state.insert(Connected{connection});
    if (auto controller = controller_.lock()) {
      controller->onOpen(connection->info().key);
    }
  }

  void Connections::onConnectionClose(ConnectionInfo connection_info) {
    auto state = entry(connections_, connection_info.key);
    if (not state or not std::holds_alternative<Connected>(*state)
        or std::get<Connected>(*state)->info() != connection_info) {
      todoPreferConnection();
    }
    state.erase();
    if (auto controller = controller_.lock()) {
      controller->onClose(connection_info.key);
    }
  }

  void Connections::onStreamAccept(ConnectionPtr connection,
                                   ProtocolId protocol_id,
                                   StreamPtr stream) {
    coroSpawn(*io_context_ptr_,
              [self{shared_from_this()},
               protocol_id,
               stream{std::move(stream)},
               connection_info{connection->info()}]() mutable -> Coro<void> {
                auto serve_it = qtils::entry(self->protocols_, protocol_id);
                if (not serve_it) {
                  co_return;
                }
                auto serve = *serve_it;
                std::ignore = CORO_WEAK_AWAIT(
                    self, serve(connection_info, std::move(stream)));
              });
  }
}  // namespace jam::snp
