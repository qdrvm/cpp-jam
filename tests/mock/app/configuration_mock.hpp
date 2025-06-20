/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <gmock/gmock.h>

#include "app/configuration.hpp"

namespace jam::app {

  class ConfigurationMock : public Configuration {
   public:
    // clang-format off
    MOCK_METHOD(const std::string&, nodeVersion, (), (const, override));
    MOCK_METHOD(const std::string&, nodeName, (), (const, override));
    MOCK_METHOD(const std::filesystem::path&, basePath, (), (const, override));
    MOCK_METHOD(const std::filesystem::path&, specFile, (), (const, override));
    MOCK_METHOD(const std::filesystem::path&, modulesDir, (), (const, override));

    MOCK_METHOD(const DatabaseConfig &, database, (), (const, override));

    MOCK_METHOD(const MetricsConfig &, metrics, (), (const, override));
    // clang-format on
  };

}  // namespace jam::app
