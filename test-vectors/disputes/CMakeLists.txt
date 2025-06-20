#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

# Copyright (c) 2019-2024 Web3 Technologies Foundation and contributors.
# This file is part of Polkadot.
# Polkadot is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

find_package(qdrvm-crates CONFIG REQUIRED)

add_test_vector(disputes tiny full)

add_test_vector_libraries(disputes
    ark_vrf::ark_vrf
    blake2b
    schnorrkel::schnorrkel
)
