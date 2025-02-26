/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lsquic.h>

#include "log/logger.hpp"

namespace jam::snp::lsquic::log_level {
  constexpr auto *emerg = "emerg";
  constexpr auto *alert = "alert";
  constexpr auto *crit = "crit";
  constexpr auto *error = "error";
  constexpr auto *warn = "warn";
  constexpr auto *notice = "notice";
  constexpr auto *info = "info";
  constexpr auto *debug = "debug";
}  // namespace jam::snp::lsquic::log_level

namespace jam::snp::lsquic {
  /**
   * Enable lsquic log.
   */
  inline void log(std::shared_ptr<soralog::Logger> log,
                  const char *level = log_level::debug) {
    static std::shared_ptr<soralog::Logger> static_log;
    static lsquic_logger_if ls_log{
        +[](void *, const char *buf, size_t len) {
          if (static_log != nullptr) {
            std::string_view message{buf, len};
            while (message.ends_with("\n")) {
              message.remove_suffix(1);
            }
            static_log->info("{}", message);
          }
          return 0;
        },
    };
    static_log = std::move(log);
    lsquic_logger_init(&ls_log, nullptr, LLTS_NONE);
    lsquic_set_log_level(level);
  }
}  // namespace jam::snp::lsquic
