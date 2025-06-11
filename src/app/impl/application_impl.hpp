/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <metrics/registry.hpp>

#include "app/application.hpp"
#include "se/subscription_fwd.hpp"

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

  /**
   * @brief RAII holder for subscription engine management
   *
   * SeHolder is responsible for managing the lifetime of subscription engine
   * components. It ensures proper initialization and cleanup of the
   * subscription system during application lifecycle.
   */
  struct SeHolder final {
    using SePtr = std::shared_ptr<Subscription>;
    SePtr se_;

    // Disable copying - subscription engine should not be copied
    SeHolder(const SeHolder &) = delete;
    SeHolder &operator=(const SeHolder &) = delete;

    // Disable moving - subscription engine should not be moved
    SeHolder(SeHolder &&) = delete;
    SeHolder &operator=(SeHolder &&) = delete;

    /**
     * @brief Constructs SeHolder with subscription engine instance
     * @param se Shared pointer to subscription engine
     */
    SeHolder(SePtr se);

    /**
     * @brief Destructor ensures proper cleanup of subscription engine
     */
    ~SeHolder();
  };

  class ApplicationImpl final : public Application {
   public:
    ApplicationImpl(std::shared_ptr<log::LoggingSystem> logsys,
                    std::shared_ptr<Configuration> config,
                    std::shared_ptr<StateManager> state_manager,
                    std::shared_ptr<Watchdog> watchdog,
                    std::shared_ptr<metrics::Exposer> metrics_exposer,
                    std::shared_ptr<clock::SystemClock> system_clock,
                    std::shared_ptr<SeHolder>);

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

}  // namespace jam::app
