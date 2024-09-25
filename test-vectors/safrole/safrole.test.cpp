/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "safrole.hpp"
#include "types.diff.hpp"
#include "vectors.hpp"

/**
 * Check safrole state transition against test vectors.
 */
GTEST_VECTORS {
  using types = Vectors::types;
  auto testcase = Vectors::read(path);
  auto [state, output] = jam::safrole::Generic<types>::transition(
      testcase->pre_state, testcase->input);
  Indent indent{1};
  EXPECT_EQ(state, testcase->post_state);
  if (state != testcase->post_state) {
    diff_m(indent, state, testcase->post_state, "state");
  }
  EXPECT_EQ(output, testcase->output);
  if (output != testcase->output) {
    diff_m(indent, output, testcase->output, "output");
  }
}
