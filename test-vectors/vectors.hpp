/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <filesystem>
#include <set>

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <qtils/read_file.hpp>
#include <scale/scale.hpp>

/**
 * Common functions for test vectors
 */
#define GTEST_VECTORS(VectorName, T)                                      \
  struct VectorName##Test : jam::test_vectors::TestT<T> {};               \
  INSTANTIATE_TEST_SUITE_P(                                               \
      VectorName,                                                         \
      VectorName##Test,                                                   \
      testing::ValuesIn([] {                                              \
        std::vector<std::pair<std::shared_ptr<T>, std::filesystem::path>> \
            params;                                                       \
        for (auto &vectors : T::vectors()) {                              \
          for (auto &path : vectors->paths) {                             \
            params.emplace_back(vectors, path);                           \
          }                                                               \
        }                                                                 \
        return params;                                                    \
      }()),                                                               \
      [](auto &&info) {                                                   \
        return jam::test_vectors::getTestName(std::get<1>(info.param));   \
      });

/**
 * Check state transition against test vectors.
 * @given `pre_state`
 * @when transition with `input`
 * @then get expected `post_state` and `output`
 */
#define GTEST_VECTORS_TEST_TRANSITION(VectorName, Namespace)        \
  using jam::test_vectors::getTestLabel;                            \
  TEST_P(VectorName##Test, Transition) {                            \
    fmt::println("Test transition for '{}'\n", getTestLabel(path)); \
    auto testcase = vectors.read(path);                             \
    auto [state, output] = Namespace::transition(                   \
        vectors.config, testcase.pre_state, testcase.input);        \
    Indent indent{1};                                               \
    EXPECT_EQ(state, testcase.post_state)                           \
        << "Actual and expected states are differ";                 \
    if (state != testcase.post_state) {                             \
      diff_m(indent, state, testcase.post_state, "state");          \
    }                                                               \
    EXPECT_EQ(output, testcase.output)                              \
        << "Actual and expected outputs are differ";                \
    if (output != testcase.output) {                                \
      diff_m(indent, output, testcase.output, "output");            \
    }                                                               \
  }

/**
 * Check state transition against test vectors.
 * @given `original` value
 * @when decode it and encode back
 * @then `actual` result has the same value as `original`
 */
#define GTEST_VECTORS_TEST_REENCODE(VectorName)                   \
  using jam::test_vectors::getTestLabel;                          \
  TEST_P(VectorName##Test, Reencode) {                            \
    fmt::println("Test reencode for '{}'\n", getTestLabel(path)); \
    auto expected = vectors.readRaw(path);                        \
    auto decoded = vectors.decode(expected);                      \
    auto reencoded = scale::encode(decoded).value();              \
    EXPECT_EQ(reencoded, expected);                               \
  }

namespace jam::test_vectors {
  inline const std::filesystem::path dir =
      std::filesystem::path{PROJECT_SOURCE_DIR} / "test-vectors/jamtestvectors";

  auto getTestLabel(const std::filesystem::path &path) {
    std::filesystem::path label;
    auto rel_path =
        std::filesystem::relative(path.parent_path(), jam::test_vectors::dir);
    auto it = rel_path.begin();
    if (it != rel_path.end()) {
      ++it;
    }
    for (; it != rel_path.end(); ++it) {
      label /= *it;
    }
    label /= path.stem();
    return label.string();
  }

  auto getTestName(const std::filesystem::path &path) {
    auto label = getTestLabel(path);
    std::string name;
    name.reserve(label.size() * 2);
    for (auto c : label) {
      if (c == '/') {
        name.append("__");
      } else if (not isalnum(c)) {
        name.push_back('_');
      } else {
        name.push_back(c);
      }
    }
    return name;
  }

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
