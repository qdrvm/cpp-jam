/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <filesystem>
#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <utils/ctor_limiters.hpp>

namespace jam::app {
  class Configuration : Singleton<Configuration> {
   public:
    using Endpoint = boost::asio::ip::tcp::endpoint;

    struct DatabaseConfig {
      std::filesystem::path directory = "db";
      size_t cache_size = 1 << 30;  // 1GiB
    };

    struct MetricsConfig {
      Endpoint endpoint;
      std::optional<bool> enabled;
    };

    Configuration();
    virtual ~Configuration() = default;

    // /// Generate yaml-file with actual config
    // virtual void generateConfigFile() const = 0;

    [[nodiscard]] virtual const std::string &nodeVersion() const;
    [[nodiscard]] virtual const std::string &nodeName() const;
    [[nodiscard]] virtual const std::filesystem::path &basePath() const;
    [[nodiscard]] virtual const std::filesystem::path &specFile() const;
    [[nodiscard]] virtual const std::filesystem::path &modulesDir() const;

    [[nodiscard]] virtual const DatabaseConfig &database() const;

    [[nodiscard]] virtual const MetricsConfig &metrics() const;

   private:
    friend class Configurator;  // for external configure

    std::string version_;
    std::string name_;
    std::filesystem::path base_path_;
    std::filesystem::path spec_file_;
    std::filesystem::path modules_dir_;

    DatabaseConfig database_;
    MetricsConfig metrics_;
  };

}  // namespace jam::app
