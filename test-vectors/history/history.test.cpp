/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "history.hpp"
#include "types.diff.hpp"
#include "vectors.hpp"

using jam::test_vectors_history::Vectors;

namespace types = jam::test_vectors_history;

bool ok = true;

/**
 * Check history state transition against test vectors.
 * @given `pre_state`
 * @when transition with `input`
 * @then get expected `post_state` and `output`
 */
void test_transition() {
  Vectors vectors;
  for (auto &path : vectors.paths) {
    auto s = path.native();
    fmt::println("{}.json", s.substr(0, s.size() - 6));
    auto testcase = vectors.read(path);
    auto [state, output] =
        jam::history::transition(testcase.pre_state, testcase.input);
    Indent indent{1};
    if (state != testcase.post_state) {
      ok = false;
      diff_m(indent, state, testcase.post_state, "state");
    }
    if (output != testcase.output) {
      ok = false;
      diff_m(indent, output, testcase.output, "output");
    }
  }
}

int main() {
  test_transition();
  fmt::println("ok");
  return 0;
}
