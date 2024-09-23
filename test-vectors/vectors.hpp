/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <qtils/read_file.hpp>
#include <scale/scale.hpp>
#include <set>

namespace jam::test_vectors {
  inline const std::filesystem::path dir =
      std::filesystem::path{PROJECT_SOURCE_DIR} / "test-vectors/jamtestvectors";

  template <typename T>
  struct Vectors {
    std::set<std::filesystem::path> paths;

    void list(const std::filesystem::path &relative) {
      for (auto &file : std::filesystem::directory_iterator{dir / relative}) {
        if (file.path().extension() == ".scale") {
          paths.emplace(file.path());
        }
      }
    }

    static auto decode(qtils::BytesIn raw) {
      scale::ScaleDecoderStream s{raw};
      auto testcase = std::make_unique<T>();
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

  /**
   * Check python generated scale encoding/decoding against test vectors.
   */
  template <typename T>
  void test_reencode(const T &vectors = {}) {
    for (auto &path : vectors.paths) {
      fmt::println("{}", path.native());
      auto expected = vectors.readRaw(path);
      auto decoded = vectors.decode(expected);
      auto reencoded = scale::encode(*decoded).value();
      if (reencoded != expected) {
        throw std::logic_error{"reencoded"};
      }
    }
  }
}  // namespace jam::test_vectors
