/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <set>

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <qtils/read_file.hpp>
#include <scale/scale.hpp>

/**
 * Common functions for test vectors
 */

#define GTEST_VECTORS(T)                                                      \
  struct Test : jam::test_vectors::TestT<T> {};                               \
  INSTANTIATE_TEST_SUITE_P(Test, Test, testing::ValuesIn([] {                 \
    std::vector<std::pair<std::shared_ptr<T>, std::filesystem::path>> params; \
    for (auto &vectors : T::vectors()) {                                      \
      for (auto &path : vectors->paths) {                                     \
        params.emplace_back(vectors, path);                                   \
      }                                                                       \
    }                                                                         \
    return params;                                                            \
  }()));

#define GTEST_VECTORS_REENCODE                       \
  TEST_P(Test, Reencode) {                           \
    auto expected = vectors.readRaw(path);           \
    auto decoded = vectors.decode(expected);         \
    auto reencoded = scale::encode(decoded).value(); \
    EXPECT_EQ(reencoded, expected);                  \
  }

namespace jam::test_vectors {
  inline const std::filesystem::path dir =
      std::filesystem::path{PROJECT_SOURCE_DIR} / "test-vectors/jamtestvectors";

  template <typename T, typename Config>
  struct VectorsT {
    Config config;
    std::set<std::filesystem::path> paths;

    VectorsT(Config config) : config{config} {}

    void list(const std::filesystem::path &relative) {
      auto ext_scale = ".scale", ext_json = ".json";
      auto use_ext = ext_scale;
      std::map<std::filesystem::path, bool> path_ok;
      for (auto &file : std::filesystem::directory_iterator{dir / relative}) {
        auto path = file.path(), ext = path.extension();
        if (ext != ext_scale and ext != ext_json) {
          continue;
        }
        path.replace_extension(use_ext);
        auto &ok = path_ok[path];
        ok = ok or ext == use_ext;
      }
      for (auto &[path, ok] : path_ok) {
        if (not ok) {
          fmt::println(
              "{}:{} warning: {} is missing, but files with other extensions "
              "are available",
              __FILE__,
              __LINE__,
              path.native());
          continue;
        }
        paths.emplace(path);
      }
    }

    auto decode(qtils::BytesIn raw) const {
      scale::ScaleDecoderStream s{raw};
      T testcase;
      decodeConfig(s, testcase, config);
      return testcase;
    }

    static auto readRaw(const std::filesystem::path &path) {
      return qtils::readBytes(path).value();
    }

    auto read(const std::filesystem::path &path) const {
      return decode(readRaw(path));
    }
  };

  template <typename T>
  struct TestT : testing::TestWithParam<
                     std::pair<std::shared_ptr<T>, std::filesystem::path>> {
    const T &vectors = *this->GetParam().first;
    const std::filesystem::path &path = this->GetParam().second;
  };
}  // namespace jam::test_vectors
