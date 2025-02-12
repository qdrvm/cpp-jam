/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdio>
#include <lsquic.h>

namespace jam::snp::lsquic {
  inline void log() {
    static lsquic_logger_if log{
        +[](void *, const char *buf, size_t len) {
          return (int)fwrite(buf, sizeof(char), len, stdout);
        },
    };
    lsquic_logger_init(&log, nullptr, LLTS_HHMMSSMS);
    lsquic_set_log_level("debug");
  }
}  // namespace jam::snp::lsquic
