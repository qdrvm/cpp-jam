/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/history/history.hpp>
#include <test-vectors/history/types.diff.hpp>
#include <test-vectors/history/vectors.hpp>

GTEST_VECTORS(jam::test_vectors_history::Vectors);

/**
 * Check history state transition against test vectors.
 * @given `pre_state`
 * @when transition with `input`
 * @then get expected `post_state` and `output`
 */
TEST_P(Test, Transition) {
  auto testcase = vectors.read(path);
  auto [state, output] =
      jam::history::transition(testcase.pre_state, testcase.input);
  Indent indent{1};
  EXPECT_EQ(state, testcase.post_state);
  if (state != testcase.post_state) {
    diff_m(indent, state, testcase.post_state, "state");
  }
  EXPECT_EQ(output, testcase.output);
  if (output != testcase.output) {
    diff_m(indent, output, testcase.output, "output");
  }
}
