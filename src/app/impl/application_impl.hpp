/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "app/application.hpp"

#include <memory>

namespace jam::app {
  class Configuration;
  class StateManager;
}  // namespace jam::app

namespace soralog {
  class Logger;
}  // namespace soralog

namespace jam::log {
  class LoggingSystem;
}  // namespace jam::log

namespace jam::app {

  class ApplicationImpl final : public Application {
   public:
    explicit ApplicationImpl(std::shared_ptr<log::LoggingSystem> logsys,
                             std::shared_ptr<Configuration> config,
                             std::shared_ptr<StateManager> state_manager);

    void run() override;

   private:
    std::shared_ptr<soralog::Logger> logger_;
    std::shared_ptr<Configuration> app_config_;
    std::shared_ptr<StateManager> state_manager_;
  };

}  // namespace jam::app
