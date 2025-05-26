// Auto-generated file

#pragma once

#include <jam_types/config.hpp>

namespace jam::test_vectors::config {

  constexpr Config dynamic {
    .validators_count = std::numeric_limits<decltype(Config::validators_count)>::max(),
    .cores_count = std::numeric_limits<decltype(Config::cores_count)>::max(),
    .epoch_length = std::numeric_limits<decltype(Config::epoch_length)>::max(),
    .max_tickets_per_block = std::numeric_limits<decltype(Config::max_tickets_per_block)>::max(),
    .tickets_per_validator = std::numeric_limits<decltype(Config::tickets_per_validator)>::max(),
    .max_blocks_history = std::numeric_limits<decltype(Config::max_blocks_history)>::max(),
    .auth_pool_max_size = std::numeric_limits<decltype(Config::auth_pool_max_size)>::max(),
    .auth_queue_size = std::numeric_limits<decltype(Config::auth_queue_size)>::max(),
    .validators_super_majority = std::numeric_limits<decltype(Config::validators_super_majority)>::max(),
    .avail_bitfield_bytes = std::numeric_limits<decltype(Config::avail_bitfield_bytes)>::max(),
  };

};
