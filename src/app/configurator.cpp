/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app/configuration.hpp"

#include <qtils/outcome.hpp>

#include "app/configurator.hpp"

namespace jam::app {

  Configurator::Configurator(int argc, const char **argv, const char **env) {
    config_ = std::make_shared<Configuration>();
  }

  std::optional<YAML::Node> Configurator::getLoggingConfig() {
    return std::nullopt;
  }

  outcome::result<std::shared_ptr<Configuration>> Configurator::calculateConfig(
      std::shared_ptr<soralog::Logger> logger) {
    return config_;
  }

}  // namespace jam::app
