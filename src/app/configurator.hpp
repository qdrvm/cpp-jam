/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <yaml-cpp/yaml.h>
#include <qtils/outcome.hpp>
#include <qtils/enum_error_code.hpp>

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
    enum class Error : uint8_t {
      CliArgsParseFailed,
      ConfigFileParseFailed,
    };

    DONT_INJECT(Configurator);

    Configurator() = delete;
    Configurator(Configurator &&) noexcept = delete;
    Configurator(const Configurator &) = delete;
    ~Configurator() = default;
    Configurator &operator=(Configurator &&) noexcept = delete;
    Configurator &operator=(const Configurator &) = delete;

    Configurator(int argc, const char **argv, const char **env);

    // Parse CLI args for help, version and config
    outcome::result<bool> step1();

    outcome::result<YAML::Node> getLoggingConfig();

    outcome::result<std::shared_ptr<Configuration>> calculateConfig(
        std::shared_ptr<soralog::Logger> logger);

  private:
    int argc_;
    const char **argv_;
    const char **env_;
    std::shared_ptr<Configuration> config_;
    std::optional<YAML::Node> config_file_;
  };

}  // namespace jam::app

OUTCOME_HPP_DECLARE_ERROR(jam::app, Configurator::Error);