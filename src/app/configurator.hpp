/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <yaml-cpp/yaml.h>
#include <boost/program_options.hpp>
#include <qtils/enum_error_code.hpp>
#include <qtils/outcome.hpp>

#include "injector/dont_inject.hpp"

namespace soralog {
  class Logger;
}  // namespace soralog

namespace jam::app {
  class Configuration;
}  // namespace jam::app

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

    // Parse remaining CLI args
    outcome::result<bool> step2();

    outcome::result<YAML::Node> getLoggingConfig();

    outcome::result<std::shared_ptr<Configuration>> calculateConfig(
        std::shared_ptr<soralog::Logger> logger);

   private:
    outcome::result<void> initGeneralConfig();
    outcome::result<void> initOpenMetricsConfig();

    int argc_;
    const char **argv_;
    const char **env_;

    std::shared_ptr<Configuration> config_;

    std::optional<YAML::Node> config_file_;
    bool file_has_warn_ = false;
    bool file_has_error_ = false;
    std::ostringstream file_errors_;

    boost::program_options::options_description cli_options_;
    boost::program_options::variables_map cli_values_map_;
  };

}  // namespace jam::app

OUTCOME_HPP_DECLARE_ERROR(jam::app, Configurator::Error);
