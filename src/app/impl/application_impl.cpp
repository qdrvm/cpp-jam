/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app/impl/application_impl.hpp"

#include <thread>
#include <unistd.h>

#include "app/configuration.hpp"
#include "app/impl/watchdog.hpp"
#include "app/state_manager.hpp"
#include "clock/clock.hpp"
#include "log/logger.hpp"
#include "metrics/histogram_timer.hpp"
#include "metrics/metrics.hpp"

namespace jam::app {

  ApplicationImpl::ApplicationImpl(
      std::shared_ptr<log::LoggingSystem> logsys,
      std::shared_ptr<Configuration> config,
      std::shared_ptr<StateManager> state_manager,
      std::shared_ptr<Watchdog> watchdog,
      std::shared_ptr<metrics::Exposer> metrics_exposer,
      std::shared_ptr<clock::SystemClock> system_clock)
      : logger_(logsys->getLogger("Application", "application")),
        app_config_(std::move(config)),
        state_manager_(std::move(state_manager)),
        watchdog_(std::move(watchdog)),
        metrics_exposer_(std::move(metrics_exposer)),
        system_clock_(std::move(system_clock)),
        metrics_registry_(metrics::createRegistry()) {
    // Metric for exposing name and version of node
    constexpr auto buildInfoMetricName = "jam_build_info";
    metrics_registry_->registerGaugeFamily(
        buildInfoMetricName,
        "A metric with a constant '1' value labeled by name, version");
    auto metric_build_info = metrics_registry_->registerGaugeMetric(
        buildInfoMetricName,
        {{"name", app_config_->nodeName()},
         {"version", app_config_->nodeVersion()}});
    metric_build_info->set(1);
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

    // Metric storing start time
    metrics::GaugeHelper("jam_process_start_time_seconds",
                         "UNIX timestamp of the moment the process started")
        ->set(system_clock_->nowSec());

    state_manager_->run();

    watchdog_->stop();

    watchdog_thread.join();
  }

}  // namespace jam::app
