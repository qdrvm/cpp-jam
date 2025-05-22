/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <blockchain/block_tree.hpp>
#include <metrics/registry.hpp>
#include <qtils/shared_ref.hpp>

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
    ApplicationImpl(qtils::SharedRef<log::LoggingSystem> logsys,
                    qtils::SharedRef<Configuration> config,
                    qtils::SharedRef<StateManager> state_manager,
                    qtils::SharedRef<Watchdog> watchdog,
                    qtils::SharedRef<metrics::Exposer> metrics_exposer,
                    qtils::SharedRef<clock::SystemClock> system_clock

                    ,
                    qtils::SharedRef<blockchain::BlockTree>
                    // qtils::SharedRef<blockchain::BlockTreeInitializer>

                    );

    void run() override;

   private:
    qtils::SharedRef<soralog::Logger> logger_;
    qtils::SharedRef<Configuration> app_config_;
    qtils::SharedRef<StateManager> state_manager_;
    qtils::SharedRef<Watchdog> watchdog_;
    qtils::SharedRef<metrics::Exposer> metrics_exposer_;
    qtils::SharedRef<clock::SystemClock> system_clock_;

    // Metrics
    std::unique_ptr<metrics::Registry> metrics_registry_;
  };

}  // namespace jam::app
