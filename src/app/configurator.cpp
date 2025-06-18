/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app/configuration.hpp"

#include <filesystem>
#include <iostream>

#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <qtils/outcome.hpp>

#include "app/build_version.hpp"
#include "app/configurator.hpp"

#include <boost/beast/core/error.hpp>

using Endpoint = boost::asio::ip::tcp::endpoint;

OUTCOME_CPP_DEFINE_CATEGORY(morum::app, Configurator::Error, e) {
  using E = morum::app::Configurator::Error;
  switch (e) {
    case E::CliArgsParseFailed:
      return "CLI Arguments parse failed";
    case E::ConfigFileParseFailed:
      return "Config file parse failed";
  }
  BOOST_UNREACHABLE_RETURN("Unknown log::Error");
}

namespace {
  template <typename T, typename Func>
  void find_argument(boost::program_options::variables_map &vm,
                     const char *name,
                     Func &&f) {
    assert(nullptr != name);
    if (auto it = vm.find(name); it != vm.end()) {
      if (it->second.defaulted()) {
        return;
      }
      std::forward<Func>(f)(it->second.as<T>());
    }
  }

  template <typename T>
  std::optional<T> find_argument(boost::program_options::variables_map &vm,
                                 const std::string &name) {
    if (auto it = vm.find(name); it != vm.end()) {
      if (!it->second.defaulted()) {
        return it->second.as<T>();
      }
    }
    return std::nullopt;
  }

  bool find_argument(boost::program_options::variables_map &vm,
                     const std::string &name) {
    if (auto it = vm.find(name); it != vm.end()) {
      if (!it->second.defaulted()) {
        return true;
      }
    }
    return false;
  }
}  // namespace

namespace morum::app {

  Configurator::Configurator(int argc, const char **argv, const char **env)
      : argc_(argc), argv_(argv), env_(env) {
    config_ = std::make_shared<Configuration>();

    config_->version_ = buildVersion();
    config_->name_ = "noname";
    config_->metrics_endpoint_ = {boost::asio::ip::address_v4::any(), 9615};
    config_->metrics_enabled_ = std::nullopt;

    namespace po = boost::program_options;

    // clang-format off

    po::options_description general_options("General options", 120, 100);
    general_options.add_options()
        ("help,h", "show this help message")
        ("version,v", "show version information")
        ("name,n", po::value<std::string>(), "set name of node")
        ("config,c", po::value<std::string>(),  "optional, filepath to load configuration from. Overrides default config values")
        ("log,l", po::value<std::vector<std::string>>(),
          "Sets a custom logging filter.\n"
          "Syntax is `<target>=<level>`, e.g. -llibp2p=off.\n"
          "Log levels (most to least verbose) are trace, debug, verbose, info, warn, error, critical, off.\n"
          "By default, all targets log `info`.\n"
          "The global log level can be set with -l<level>.")
        ;

    po::options_description metrics_options("Metric options");
    metrics_options.add_options()
    ("prometheus-disable", "set to disable OpenMetrics")
    ("prometheus-host", po::value<std::string>(), "address for OpenMetrics over HTTP")
    ("prometheus-port", po::value<uint16_t>(), "port for OpenMetrics over HTTP")
        ;

    // clang-format on

    cli_options_
        .add(general_options)  //
        .add(metrics_options);
  }

  outcome::result<bool> Configurator::step1() {
    namespace po = boost::program_options;
    namespace fs = std::filesystem;

    po::options_description options;
    options.add_options()("help,h", "show help")("version,v", "show version")(
        "config,c", po::value<std::string>(), "config-file path");

    po::variables_map vm;

    // first-run parse to read only general options and to lookup for "help",
    // "config" and "version". all the rest options are ignored
    try {
      po::parsed_options parsed = po::command_line_parser(argc_, argv_)
                                      .options(options)
                                      .allow_unregistered()
                                      .run();
      po::store(parsed, vm);
      po::notify(vm);
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << '\n'
                << "Try run with option '--help' for more information\n";
      return Error::CliArgsParseFailed;
    }

    if (vm.contains("help")) {
      std::cout << "Morum version " << buildVersion() << '\n';
      std::cout << cli_options_ << '\n';
      return true;
    }

    if (vm.contains("version")) {
      std::cout << "Morum version " << buildVersion() << '\n';
      return true;
    }

    if (vm.contains("config")) {
      auto path = vm["config"].as<std::string>();
      try {
        config_file_ = YAML::LoadFile(path);
      } catch (const std::exception &exception) {
        std::cerr << "Error: Can't parse file "
                  << std::filesystem::weakly_canonical(path) << ": "
                  << exception.what() << "\n"
                  << "Option --config must be path to correct yaml-file\n"
                  << "Try run with option '--help' for more information\n";
        return Error::ConfigFileParseFailed;
      }
    }

    return false;
  }

  outcome::result<bool> Configurator::step2() {
    namespace po = boost::program_options;
    namespace fs = std::filesystem;

    try {
      // second-run parse to gather all known options
      // with reporting about any unrecognized input
      po::parsed_options parsed =
          po::command_line_parser(argc_, argv_).options(cli_options_).run();
      po::store(parsed, cli_values_map_);
      po::notify(cli_values_map_);
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << '\n'
                << "Try run with option '--help' for more information\n";
      return Error::CliArgsParseFailed;
    }

    return false;
  }

  outcome::result<YAML::Node> Configurator::getLoggingConfig() {
    auto logging = (*config_file_)["logging"];
    if (logging.IsDefined()) {
      return logging;
    }
    return YAML::Node{};  // TODO return default logging config
  }

  outcome::result<std::shared_ptr<Configuration>> Configurator::calculateConfig(
      std::shared_ptr<soralog::Logger> logger) {
    OUTCOME_TRY(initGeneralConfig());
    OUTCOME_TRY(initOpenMetricsConfig());

    return config_;
  }

  outcome::result<void> Configurator::initGeneralConfig() {
    if (config_file_.has_value()) {
      auto section = (*config_file_)["general"];
      if (section.IsDefined()) {
        if (section.IsMap()) {
          auto name = section["name"];
          if (name.IsDefined()) {
            if (name.IsScalar()) {
              auto value = name.as<std::string>();
              config_->name_ = value;
            } else {
              file_errors_ << "E: Value 'general.name' must be scalar\n";
              file_has_error_ = true;
            }
          }
        } else {
          file_errors_ << "E: Section 'general' defined, but is not scalar\n";
          file_has_error_ = true;
        }
      }
    }

    bool fail;

    fail = false;
    find_argument<std::string>(
        cli_values_map_, "name", [&](const std::string &value) {
          config_->name_ = value;
        });
    if (fail) {
      return Error::CliArgsParseFailed;
    }

    return outcome::success();
  }

  outcome::result<void> Configurator::initOpenMetricsConfig() {
    if (config_file_.has_value()) {
      auto section = (*config_file_)["metrics"];
      if (section.IsDefined()) {
        if (section.IsMap()) {
          auto enabled = section["enabled"];
          if (enabled.IsDefined()) {
            if (enabled.IsScalar()) {
              auto value = enabled.as<std::string>();
              if (value == "true") {
                config_->metrics_enabled_ = true;
              } else if (value == "false") {
                config_->metrics_enabled_ = false;
              } else {
                file_errors_
                    << "E: Value 'network.metrics.enabled' has wrong value. "
                       "Expected 'true' or 'false'\n";
                file_has_error_ = true;
              }
            } else {
              file_errors_ << "E: Value 'metrics.enabled' must be scalar\n";
              file_has_error_ = true;
            }
          }

          auto host = section["host"];
          if (host.IsDefined()) {
            if (host.IsScalar()) {
              auto value = host.as<std::string>();
              boost::beast::error_code ec;
              auto address = boost::asio::ip::address::from_string(value, ec);
              if (!ec) {
                config_->metrics_endpoint_ = {
                    address, config_->metrics_endpoint_.port()};
                if (not config_->metrics_enabled_.has_value()) {
                  config_->metrics_enabled_ = true;
                }
              } else {
                file_errors_ << "E: Value 'network.metrics.host' defined, "
                                "but has invalid value\n";
              }
            } else {
              file_errors_ << "E: Value 'network.metrics.host' defined, "
                              "but is not scalar\n";
              file_has_error_ = true;
            }
          }

          auto port = section["port"];
          if (port.IsDefined()) {
            if (port.IsScalar()) {
              auto value = port.as<ssize_t>();
              if (value > 0 and value <= 65535) {
                config_->metrics_endpoint_ = {
                    config_->metrics_endpoint_.address(),
                    static_cast<uint16_t>(value)};
                if (not config_->metrics_enabled_.has_value()) {
                  config_->metrics_enabled_ = true;
                }
              } else {
                file_errors_ << "E: Value 'network.metrics.port' defined, "
                                "but has invalid value\n";
                file_has_error_ = true;
              }
            } else {
              file_errors_ << "E: Value 'network.metrics.port' defined, "
                              "but is not scalar\n";
              file_has_error_ = true;
            }
          }

        } else {
          file_errors_ << "E: Section 'metrics' defined, but is not map\n";
          file_has_error_ = true;
        }
      }
    }

    bool fail;

    fail = false;
    find_argument<std::string>(
        cli_values_map_, "prometheus-host", [&](const std::string &value) {
          boost::beast::error_code ec;
          auto address = boost::asio::ip::address::from_string(value, ec);
          if (!ec) {
            config_->metrics_endpoint_ = {address,
                                          config_->metrics_endpoint_.port()};
            if (not config_->metrics_enabled_.has_value()) {
              config_->metrics_enabled_ = true;
            }
          } else {
            std::cerr << "Option --prometheus-host has invalid value\n"
                      << "Try run with option '--help' for more information\n";
            fail = true;
          }
        });
    if (fail) {
      return Error::CliArgsParseFailed;
    }

    fail = false;
    find_argument<uint32_t>(
        cli_values_map_, "prometheus-port", [&](const uint32_t &value) {
          if (value > 0 and value <= 65535) {
            config_->metrics_endpoint_ = {config_->metrics_endpoint_.address(),
                                          static_cast<uint16_t>(value)};
            if (not config_->metrics_enabled_.has_value()) {
              config_->metrics_enabled_ = true;
            }
          } else {
            std::cerr << "Option --prometheus-port has invalid value\n"
                      << "Try run with option '--help' for more information\n";
            fail = true;
          }
        });
    if (fail) {
      return Error::CliArgsParseFailed;
    }

    if (find_argument(cli_values_map_, "prometheus-disabled")) {
      config_->metrics_enabled_ = false;
    };
    if (not config_->metrics_enabled_.has_value()) {
      config_->metrics_enabled_ = false;
    }

    return outcome::success();
  }

}  // namespace morum::app
