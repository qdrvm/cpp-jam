/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/types.tmp.hpp>

#include <qtils/byte_view.hpp>

namespace jam::crypto {

  Hash64 make_twox64(qtils::ByteView buf);

  Hash128 make_twox128(qtils::ByteView buf);

  Hash256 make_twox256(qtils::ByteView buf);

}  // namespace jam::crypto
