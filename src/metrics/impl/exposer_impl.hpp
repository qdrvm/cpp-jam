/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "metrics/exposer.hpp"

#include <thread>

namespace jam::app {
  class StateManager;
  class Configuration;
}  // namespace jam::app

namespace soralog {
  class Logger;
}  // namespace soralog

namespace jam::log {
  class LoggingSystem;
}  // namespace jam::log

namespace jam::metrics {

  class ExposerImpl : public Exposer,
                      public std::enable_shared_from_this<ExposerImpl> {
   public:
    ExposerImpl(std::shared_ptr<log::LoggingSystem> logsys,
                std::shared_ptr<app::StateManager> state_manager,
                std::shared_ptr<app::Configuration> config,
                std::shared_ptr<metrics::Handler> handler,
                Session::Configuration session_config);

    ~ExposerImpl() override = default;

    bool prepare() override;
    bool start() override;
    void stop() override;

   private:
    void acceptOnce();

    std::shared_ptr<soralog::Logger> logger_;
    std::shared_ptr<log::LoggingSystem> logsys_;
    std::shared_ptr<Context> context_;
    std::shared_ptr<app::Configuration> config_;
    const Session::Configuration session_config_;

    std::unique_ptr<Acceptor> acceptor_;

    std::shared_ptr<Session> new_session_;

    std::unique_ptr<std::thread> thread_;
  };

}  // namespace jam::metrics
