/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <fmt/core.h>
#include <qtils/tagged.hpp>

template <typename T, typename U>
struct fmt::formatter<qtils::Tagged<T, U>> : formatter<T> {};
