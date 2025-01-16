/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "metrics/impl/exposer_impl.hpp"

#include <thread>

#include "app/state_manager.hpp"
#include "log/logger.hpp"
#include "metrics/exposer.hpp"
#include "metrics/impl/session_impl.hpp"
#include "utils/tuner.hpp"
#include "metrics/metrics.hpp"

namespace jam::metrics {
  ExposerImpl::ExposerImpl(std::shared_ptr<log::LoggingSystem> logsys,
                           std::shared_ptr<app::StateManager> state_manager,
                           Exposer::Configuration exposer_config,
                           std::shared_ptr<metrics::Handler> handler,
                           Session::Configuration session_config)
      : logger_{logsys->getLogger("OpenMetrics", "metrics")},
        logsys_{std::move(logsys)},
        context_{std::make_shared<Context>()},
        config_{std::move(exposer_config)},
        session_config_{session_config} {

    auto registry = metrics::createRegistry();
    registry->setHandler(*handler.get());
    setHandler(handler);

    state_manager->takeControl(*this);
  }

  bool ExposerImpl::prepare() {
    try {
      acceptor_ = jam::api::acceptOnFreePort(
          context_, config_.endpoint, jam::api::kDefaultPortTolerance, logger_);
    } catch (const boost::wrapexcept<boost::system::system_error> &exception) {
      SL_CRITICAL(
          logger_, "Failed to prepare a listener: {}", exception.what());
      return false;
    } catch (const std::exception &exception) {
      SL_CRITICAL(
          logger_, "Exception when preparing a listener: {}", exception.what());
      return false;
    }

    boost::system::error_code ec;
    acceptor_->set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec) {
      logger_->error("Failed to set `reuse address` option to acceptor");
      return false;
    }
    return true;
  }

  bool ExposerImpl::start() {
    BOOST_ASSERT(acceptor_);

    if (!acceptor_->is_open()) {
      logger_->error("error: trying to start on non opened acceptor");
      return false;
    }

    logger_->info("Listening for new connections on {}:{}",
                  config_.endpoint.address().to_string(),
                  acceptor_->local_endpoint().port());
    acceptOnce();

    thread_ = std::make_unique<std::thread>([context = context_] {
      soralog::util::setThreadName("metric-exposer");
      context->run();
    });

    return true;
  }

  void ExposerImpl::stop() {
    if (acceptor_) {
      acceptor_->cancel();
    }
    context_->stop();
    if (thread_) {
      thread_->join();
      thread_.reset();
    }
  }

  void ExposerImpl::acceptOnce() {
    new_session_ =
        std::make_shared<SessionImpl>(logsys_, *context_, session_config_);
    new_session_->connectOnRequest(
        [handler = handler_.get()](auto request, auto session) {
          handler->onSessionRequest(std::move(request), std::move(session));
        });
    auto on_accept = [wp{weak_from_this()}](boost::system::error_code ec) {
      if (auto self = wp.lock()) {
        if (not ec) {
          self->new_session_->start();
        }

        if (self->acceptor_->is_open()) {
          // continue to accept until acceptor is ready
          self->acceptOnce();
        }
      }
    };

    acceptor_->async_accept(new_session_->socket(), std::move(on_accept));
  }
}  // namespace jam::metrics
