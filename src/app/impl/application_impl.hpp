/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <metrics/registry.hpp>
#include <qtils/strict_sptr.hpp>

#include "app/application.hpp"

namespace jam {
  class Watchdog;
}  // namespace jam

namespace jam::app {
  class Configuration;
  class StateManager;
}  // namespace jam::app

namespace jam::clock {
  class SystemClock;
}  // namespace jam::clock

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
    ApplicationImpl(qtils::StrictSharedPtr<log::LoggingSystem> logsys,
                    qtils::StrictSharedPtr<Configuration> config,
                    qtils::StrictSharedPtr<StateManager> state_manager,
                    qtils::StrictSharedPtr<Watchdog> watchdog,
                    qtils::StrictSharedPtr<metrics::Exposer> metrics_exposer,
                    qtils::StrictSharedPtr<clock::SystemClock> system_clock);

    void run() override;

   private:
    qtils::StrictSharedPtr<soralog::Logger> logger_;
    qtils::StrictSharedPtr<Configuration> app_config_;
    qtils::StrictSharedPtr<StateManager> state_manager_;
    qtils::StrictSharedPtr<Watchdog> watchdog_;
    qtils::StrictSharedPtr<metrics::Exposer> metrics_exposer_;
    qtils::StrictSharedPtr<clock::SystemClock> system_clock_;

    // Metrics
    std::unique_ptr<metrics::Registry> metrics_registry_;
  };

}  // namespace jam::app
