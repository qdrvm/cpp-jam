/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <yaml-cpp/yaml.h>
#include <qtils/outcome.hpp>

#include "injector/dont_inject.hpp"

namespace soralog {
  class Logger;
}  // namespace soralog

namespace jam::app {
  class Configuration;
}

namespace jam::app {

  class Configurator final {
   public:
    DONT_INJECT(Configurator);

    Configurator() = delete;
    Configurator(Configurator &&) noexcept = delete;
    Configurator(const Configurator &) = delete;
    ~Configurator() = default;
    Configurator &operator=(Configurator &&) noexcept = delete;
    Configurator &operator=(const Configurator &) = delete;

    Configurator(int argc, const char **argv, const char **env);

    std::optional<YAML::Node> getLoggingConfig();

    outcome::result<std::shared_ptr<Configuration>> calculateConfig(
        std::shared_ptr<soralog::Logger> logger);

  private:
    std::shared_ptr<Configuration> config_;
  };

}  // namespace jam::app
