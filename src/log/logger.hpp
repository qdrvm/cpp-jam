/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <sstream>

#include <qtils/outcome.hpp>
#include <qtils/enum_error_code.hpp>
#include <soralog/level.hpp>
#include <soralog/logger.hpp>
#include <soralog/logging_system.hpp>
#include <soralog/macro.hpp>

#include "utils/ctor_limiters.hpp"
#include "injector/dont_inject.hpp"

namespace morum::log {
  using soralog::Level;

  enum class Error : uint8_t { WRONG_LEVEL = 1, WRONG_GROUP, WRONG_LOGGER };

  outcome::result<Level> str2lvl(std::string_view str);

  void setLoggingSystem(std::weak_ptr<soralog::LoggingSystem> logging_system);

  inline static std::string defaultGroupName{"morum"};

  class LoggingSystem : public Singleton<LoggingSystem> {
   public:
    DONT_INJECT(LoggingSystem);

    LoggingSystem(std::shared_ptr<soralog::LoggingSystem> logging_system);

    void tuneLoggingSystem(const std::vector<std::string> &cfg);

    void doLogRotate() const {
      logging_system_->callRotateForAllSinks();
    }

    [[nodiscard]]  //
    auto
    getLogger(const std::string &logger_name,
              const std::string &group_name) const {
      return logging_system_->getLogger(logger_name, group_name);
    }

    [[deprecated("Don't use hard coded level in production code")]]  //
    [[nodiscard]]                                                    //
    auto
    createLogger(const std::string &logger_name,
                 const std::string &group_name,
                 Level level) const {
      return logging_system_->getLogger(logger_name, group_name, level);
    }

    [[nodiscard]]
    bool setLevelOfGroup(const std::string &group_name, Level level) const {
      return logging_system_->setLevelOfGroup(group_name, level);
    }

    [[nodiscard]]
    bool resetLevelOfGroup(const std::string &group_name) const {
      return logging_system_->resetLevelOfGroup(group_name);
    }

    [[nodiscard]]
    bool setLevelOfLogger(const std::string &logger_name, Level level) const {
      return logging_system_->setLevelOfLogger(logger_name, level);
    }

    [[nodiscard]]
    bool resetLevelOfLogger(const std::string &logger_name) const {
      return logging_system_->resetLevelOfLogger(logger_name);
    }

   private:
    std::shared_ptr<soralog::LoggingSystem> logging_system_;
  };

}  // namespace morum::log

OUTCOME_HPP_DECLARE_ERROR(morum::log, Error);
