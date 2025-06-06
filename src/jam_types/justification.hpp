/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/types.tmp.hpp>
#include <qtils/byte_vec.hpp>
#include <qtils/tagged.hpp>

namespace jam {

  using Justification = qtils::Tagged<qtils::ByteVec, struct JustificationTag>;

}
