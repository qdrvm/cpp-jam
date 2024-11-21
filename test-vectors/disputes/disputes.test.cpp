/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/disputes/disputes.hpp>
#include <test-vectors/disputes/types.diff.hpp>
#include <test-vectors/disputes/vectors.hpp>

GTEST_VECTORS(Disputes, jam::test_vectors_disputes::Vectors);

GTEST_VECTORS_TEST_TRANSITION(Disputes, jam::disputes);
