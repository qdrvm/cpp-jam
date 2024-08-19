/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <qtils/read_file.hpp>
#include <set>

#include "types.scale.hpp"

namespace jam::test_vectors_safrole {
  template <bool is_full>
  struct Vectors {
    using types = std::conditional_t<is_full, full, tiny>;

    std::string type = is_full ? "full" : "tiny";
    std::filesystem::path dir = std::filesystem::path{PROJECT_SOURCE_DIR}
        / "test-vectors/jamtestvectors/safrole" / type;
    std::set<std::filesystem::path> paths;

    Vectors() {
      for (auto &file : std::filesystem::directory_iterator{dir}) {
        if (file.path().extension() == ".scale") {
          paths.emplace(file.path());
        }
      }
    }

    static auto decode(qtils::BytesIn raw) {
      scale::ScaleDecoderStream s{raw};
      auto testcase = std::make_unique<typename types::Testcase>();
      s >> *testcase;
      return testcase;
    }

    static auto readRaw(const std::filesystem::path &path) {
      return qtils::readBytes(path).value();
    }

    static auto read(const std::filesystem::path &path) {
      return decode(readRaw(path));
    }
  };
}  // namespace jam::test_vectors_safrole
