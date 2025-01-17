/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app/impl/application_impl.hpp"

#include <unistd.h>
#include <thread>

#include <soralog/macro.hpp>

#include "app/configuration.hpp"
#include "app/state_manager.hpp"
#include "log/logger.hpp"

// #include "application/impl/util.hpp"
// #include "application/modes/precompile_wasm.hpp"
// #include "application/modes/print_chain_info_mode.hpp"
// #include "application/modes/recovery_mode.hpp"
// #include "injector/application_injector.hpp"
#include "metrics/metrics.hpp"
// #include "parachain/pvf/secure_mode_precheck.hpp"
// #include "telemetry/service.hpp"

#include "app/impl/watchdog.hpp"

namespace jam::app {

  ApplicationImpl::ApplicationImpl(
      std::shared_ptr<log::LoggingSystem> logsys,
      std::shared_ptr<Configuration> config,
      std::shared_ptr<StateManager> state_manager,
      std::shared_ptr<Watchdog> watchdog,
      std::shared_ptr<metrics::Exposer> metrics_exposer
      //,std::shared_ptr<clock::SystemClock> system_clock
      )
      : logger_(logsys->getLogger("Application", "application")),
        app_config_(std::move(config)),
        state_manager_(std::move(state_manager)),
        watchdog_(std::move(watchdog)),
        metrics_exposer_(std::move(metrics_exposer)),
        metrics_registry_(metrics::createRegistry()) {
    constexpr auto exampleMetricName = "jam_metric";

    // Register metrics
    metrics_registry_->registerGaugeFamily(exampleMetricName, "Example metric");
    metric_highest_round_ =
        metrics_registry_->registerGaugeMetric(exampleMetricName);
    metric_highest_round_->set(0);
  }

  void ApplicationImpl::run() {
    logger_->info("Start as node version '{}' named as '{}' with PID {}",
                  app_config_->nodeVersion(),
                  app_config_->nodeName(),
                  getpid());

    std::thread watchdog_thread([this] {
      soralog::util::setThreadName("watchdog");
      watchdog_->checkLoop(kWatchdogDefaultTimeout);
    });

    state_manager_->atShutdown([this] { watchdog_->stop(); });

    {  // Metrics
      auto metrics_registry = metrics::createRegistry();

      constexpr auto startTimeMetricName = "jam_process_start_time_seconds";
      metrics_registry->registerGaugeFamily(
          startTimeMetricName,
          "UNIX timestamp of the moment the process started");
      auto metric_start_time =
          metrics_registry->registerGaugeMetric(startTimeMetricName);
      metric_start_time->set(123);  // system_clock->nowUint64());

      constexpr auto buildInfoMetricName = "jam_build_info";
      metrics_registry->registerGaugeFamily(
          buildInfoMetricName,
          "A metric with a constant '1' value labeled by name, version");
      auto metric_build_info = metrics_registry->registerGaugeMetric(
          buildInfoMetricName,
          {{"name", app_config_->nodeName()},
           {"version", app_config_->nodeVersion()}});
      metric_build_info->set(1);
    }

    state_manager_->run();

    watchdog_->stop();

    watchdog_thread.join();
  }

}  // namespace jam::app
