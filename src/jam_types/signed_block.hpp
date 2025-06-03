/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace jam {

  struct SignedBlock {
    Block message;
    qtils::ByteArr<32> signature;
  };

}  // namespace jam
