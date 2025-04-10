/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "app/application.hpp"

#include <memory>

#include <metrics/registry.hpp>

namespace morum {
  class Watchdog;
}  // namespace morum

namespace morum::app {
  class Configuration;
  class StateManager;
}  // namespace morum::app

namespace morum::clock {
  class SystemClock;
}  // namespace morum::clock

namespace soralog {
  class Logger;
}  // namespace soralog

namespace morum::log {
  class LoggingSystem;
}  // namespace morum::log

namespace morum::metrics {
  class Registry;
  class Gauge;
  class Exposer;
}  // namespace morum::metrics

namespace morum::app {

  class ApplicationImpl final : public Application {
   public:
    ApplicationImpl(std::shared_ptr<log::LoggingSystem> logsys,
                    std::shared_ptr<Configuration> config,
                    std::shared_ptr<StateManager> state_manager,
                    std::shared_ptr<Watchdog> watchdog,
                    std::shared_ptr<metrics::Exposer> metrics_exposer,
                    std::shared_ptr<clock::SystemClock> system_clock);

    void run() override;

   private:
    std::shared_ptr<soralog::Logger> logger_;
    std::shared_ptr<Configuration> app_config_;
    std::shared_ptr<StateManager> state_manager_;
    std::shared_ptr<Watchdog> watchdog_;
    std::shared_ptr<metrics::Exposer> metrics_exposer_;
    std::shared_ptr<clock::SystemClock> system_clock_;

    // Metrics
    std::unique_ptr<metrics::Registry> metrics_registry_;
  };

}  // namespace morum::app
