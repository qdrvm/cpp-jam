#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <iterator>

#include <nlohmann/json.hpp>
#include <qtils/assert.hpp>

#include <morum/archive_backend.hpp>
#include <morum/calculate_root_in_memory.hpp>
#include <morum/merkle_tree.hpp>
#include <morum/common.hpp>
#include <qtils/byte_view.hpp>

uint8_t unhex(char c) {
  if (isdigit(c)) {
    return c - '0';
  }
  return c - 'a' + 10;
}

char hex(uint8_t i) {
  QTILS_ASSERT_LESS(i, 16);
  if (i < 10) {
    return i + '0';
  }
  return i + 'a' - 10;
}

template <std::output_iterator<unsigned char> It>
void unhex(std::string_view hex, It it) {
  QTILS_ASSERT_EQ(hex.size() % 2, 0);
  for (size_t i = 0; i < hex.size(); i += 2) {
    *it = unhex(hex[i + 1]) | (unhex(hex[i]) << 4);
    it++;
  }
}

std::string hex(qtils::ByteView bytes) {
  std::string s(bytes.size() * 2, 0);
  for (size_t i = 0; i < bytes.size(); i++) {
    s[i * 2] = hex(bytes[i] >> 4);
    s[i * 2 + 1] = hex(bytes[i] & 0x0F);
  }
  return s;
}

int main() {
  auto file_path =
      std::filesystem::path(std::source_location::current().file_name())
          .parent_path()
      / "../../test-vectors/jamtestvectors/trie/trie.json";
  std::ifstream test_cases_file{file_path};
  if (!test_cases_file.good()) {
    std::cerr
        << "Test vectors are not found; Try running git submodule update --init.\n";
    return -1;
  }
  auto test_cases = nlohmann::json::parse(test_cases_file);

  // to start from nth case if needed
  size_t start_from{0};
  size_t test_idx{start_from};
  int status = 0;
  for (auto test_case : test_cases | std::views::drop(start_from)) {
    morum::MerkleTree tree{std::make_unique<morum::FlatPagedNodeStorage>(),
        std::make_shared<morum::NoopNodeLoader>()};
    std::vector<std::pair<morum::Hash32, qtils::ByteVec>> state;
    for (auto &[k, v] : test_case["input"].items()) {
      morum::Hash32 key;
      unhex(k, static_cast<unsigned char *>(key.data()));
      auto value_str = v.get<std::string>();
      qtils::ByteVec value(value_str.size() / 2);
      unhex(value_str, value.data());
      state.push_back(std::pair{key, value});
      [[maybe_unused]] auto res =
          tree.set(key, qtils::ByteVec{value}).has_value();
      QTILS_ASSERT(res);
    }
    std::ranges::sort(state,
        morum::TrieKeyOrder{},
        [](const auto &pair) -> const morum::Hash32 & { return pair.first; });
    for (auto &[k, v] : state) {
      std::print(std::cerr, "{}\n", hex(k));
    }
    morum::Hash32 expected_root;
    auto expected_output_str = test_case["output"].get<std::string>();
    unhex(expected_output_str, expected_root.data());
    auto root = morum::calculate_root_in_memory(state);
    std::cerr << "#" << test_idx << " - ";
    if (!std::ranges::equal(root, expected_root)) {
      std::cerr << "fail: " << hex({root.data(), 32})
                << " != " << expected_output_str << "\n";
      status = -1;
    } else {
      std::cerr << "success\n";
    }
    QTILS_ASSERT_RANGE_EQ(tree.calculate_hash(), expected_root);
    test_idx++;
  }
  return status;
}
