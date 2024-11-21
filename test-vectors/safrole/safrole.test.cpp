/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/safrole/safrole.hpp>
#include <test-vectors/safrole/types.diff.hpp>
#include <test-vectors/safrole/vectors.hpp>

GTEST_VECTORS(Safrole, jam::test_vectors_safrole::Vectors);

GTEST_VECTORS_TEST_TRANSITION(Safrole, jam::safrole);
