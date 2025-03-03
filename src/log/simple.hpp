/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <soralog/impl/configurator_from_yaml.hpp>

#include "log/logger.hpp"

namespace jam::log {
  std::shared_ptr<LoggingSystem> simpleLoggingSystem() {
    std::string yaml = R"(
    sinks:
      - name: console
        type: console
        capacity: 4
        latency: 0
    groups:
      - name: main
        sink: console
        level: info
        is_fallback: true
    )";
    auto logsys = std::make_shared<soralog::LoggingSystem>(
        std::make_shared<soralog::ConfiguratorFromYAML>(
            std::shared_ptr<soralog::Configurator>(nullptr), yaml));
    if (auto r = logsys->configure().message; not r.empty()) {
      fmt::println(stderr, "soralog error: {}", r);
    }
    return std::make_shared<LoggingSystem>(logsys);
  }
}  // namespace jam::log
