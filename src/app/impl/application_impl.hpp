/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "app/application.hpp"

#include <memory>
#include <metrics/registry.hpp>

namespace jam {
  class Watchdog;
}  // namespace jam

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

namespace jam::metrics {
  class Registry;
  class Gauge;
  class Exposer;
}  // namespace jam::metrics

namespace jam::app {

  class ApplicationImpl final : public Application {
   public:
    ApplicationImpl(std::shared_ptr<log::LoggingSystem> logsys,
                    std::shared_ptr<Configuration> config,
                    std::shared_ptr<StateManager> state_manager,
                    std::shared_ptr<Watchdog> watchdog,
                    std::shared_ptr<metrics::Exposer> metrics_exposer
                    );

    void run() override;

   private:
    std::shared_ptr<soralog::Logger> logger_;
    std::shared_ptr<Configuration> app_config_;
    std::shared_ptr<StateManager> state_manager_;
    std::shared_ptr<Watchdog> watchdog_;
    std::shared_ptr<metrics::Exposer> metrics_exposer_;

    // Metrics
    std::unique_ptr<metrics::Registry> metrics_registry_;
    metrics::Gauge *metric_highest_round_;
  };

}  // namespace jam::app
