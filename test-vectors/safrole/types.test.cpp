/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <map>
#include <qtils/read_file.hpp>

#include "types.hpp"
#include "types.scale.hpp"

template <bool full>
void test_dir() {
  using types = std::conditional_t<full,
      jam::test_vectors_safrole::full,
      jam::test_vectors_safrole::tiny>;
  auto type = full ? "full" : "tiny";
  auto dir = std::filesystem::path{PROJECT_SOURCE_DIR}
      / "test-vectors/jamtestvectors/safrole" / type;
  constexpr std::string_view kScale = ".scale";
  using Files = std::map<std::string, qtils::Bytes>;
  Files files_scale;
  auto push_if =
      [](const std::filesystem::path &path, Files &to, std::string_view ext) {
        auto name = path.filename().native();
        if (name.ends_with(ext)) {
          to.emplace(name.substr(0, name.size() - ext.size()),
              qtils::readBytes(path).value());
        }
      };
  for (auto &file : std::filesystem::directory_iterator{dir}) {
    push_if(file.path(), files_scale, kScale);
  }
  for (auto &[name, scale] : files_scale) {
    fmt::println("{}/{}.scale", type, name);
    auto decoded = std::make_unique<typename types::Testcase>();
    scale::ScaleDecoderStream s{scale};
    s >> *decoded;
    auto reencoded = scale::encode(*decoded).value();
    if (reencoded != scale) {
      throw std::logic_error{"reencoded"};
    }
  }
}

int main() {
  test_dir<false>();
  test_dir<true>();
  fmt::println("ok");
  return 0;
}
