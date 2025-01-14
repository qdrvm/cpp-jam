/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

namespace jam::app {

  class Configuration {
   public:
    Configuration() = default;
    Configuration(Configuration &&) noexcept = delete;
    Configuration(const Configuration &) = delete;
    virtual ~Configuration() = default;
    Configuration &operator=(Configuration &&) noexcept = delete;
    Configuration &operator=(const Configuration &) = delete;

    // /// Generate yaml-file with actual config
    // virtual void generateConfigFile() const = 0;

    std::string_view nodeVersion() const;
    std::string_view nodeName() const;

   private:
    std::string_view _version;
    std::string_view _name;
  };

}  // namespace jam::app
