/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdio>
#include <lsquic.h>

namespace jam::snp::lsquic {
  /**
   * Enable lsquic log.
   *
   * Possible levels: "emerg", "alert", "crit", "error", "warn", "notice",
   * "info", "debug".
   */
  inline void log(const char *level = "debug") {
    static lsquic_logger_if log{
        +[](void *, const char *buf, size_t len) {
          return (int)fwrite(buf, sizeof(char), len, stdout);
        },
    };
    static auto init = [] {
      lsquic_logger_init(&log, nullptr, LLTS_HHMMSSMS);
      return 0;
    }();
    lsquic_set_log_level(level);
  }
}  // namespace jam::snp::lsquic
