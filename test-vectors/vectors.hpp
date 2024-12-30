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
#include <qtils/test/outcome.hpp>
#include <test-vectors/config-types-scale.hpp>

/**
 * Common functions for test vectors
 */
#define GTEST_VECTORS(VectorName, NsPart)                                 \
  struct VectorName##Test                                                 \
      : jam::test_vectors::TestT<jam::test_vectors::NsPart::Vectors> {};  \
  INSTANTIATE_TEST_SUITE_P(                                               \
      VectorName,                                                         \
      VectorName##Test,                                                   \
      testing::ValuesIn([] {                                              \
        using T = jam::test_vectors::NsPart::Vectors;                     \
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
#define GTEST_VECTORS_TEST_TRANSITION(VectorName, NsPart)                    \
  TEST_P(VectorName##Test, Transition) {                                     \
    using jam::test_vectors::getTestLabel;                                   \
    fmt::println("Test transition for '{}'\n", getTestLabel(path));          \
                                                                             \
    ASSERT_OUTCOME_SUCCESS(raw_data, qtils::readBytes(path));                \
                                                                             \
    ASSERT_OUTCOME_SUCCESS(testcase,                                         \
                           jam::decode<jam::test_vectors::NsPart::TestCase>( \
                               raw_data, vectors.config));                   \
                                                                             \
    auto [state, output] = jam::NsPart::transition(                          \
        vectors.config, testcase.pre_state, testcase.input);                 \
    Indent indent{1};                                                        \
    EXPECT_EQ(state, testcase.post_state)                                    \
        << "Actual and expected states are differ";                          \
    if (state != testcase.post_state) {                                      \
      diff_m(indent, state, testcase.post_state, "state");                   \
    }                                                                        \
    EXPECT_EQ(output, testcase.output)                                       \
        << "Actual and expected outputs are differ";                         \
    if (output != testcase.output) {                                         \
      diff_m(indent, output, testcase.output, "output");                     \
    }                                                                        \
  }

/**
 * Check state transition against test vectors.
 * @given `original` value
 * @when decode it and encode back
 * @then `actual` result has the same value as `original`
 */
#define GTEST_VECTORS_TEST_REENCODE(VectorName, NsPart)                      \
  TEST_P(VectorName##Test, Reencode) {                                       \
    using jam::test_vectors::getTestLabel;                                   \
    fmt::println("Test reencode for '{}'\n", getTestLabel(path));            \
                                                                             \
    ASSERT_OUTCOME_SUCCESS(raw_data, qtils::readBytes(path));                \
    const auto &original = raw_data;                                         \
                                                                             \
    ASSERT_OUTCOME_SUCCESS(decoded,                                          \
                           jam::decode<jam::test_vectors::NsPart::TestCase>( \
                               original, vectors.config));                   \
                                                                             \
    ASSERT_OUTCOME_SUCCESS(reencoded, jam::encode(decoded, vectors.config)); \
                                                                             \
    EXPECT_EQ(reencoded, original);                                          \
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
      auto ext_bin = ".bin", ext_json = ".json", ext_scale = ".scale";
      auto use_ext = ext_bin;
      std::map<std::filesystem::path, bool> path_ok;
      for (auto &file : std::filesystem::directory_iterator{dir / relative}) {
        auto path = file.path(), ext = path.extension();
        if (ext != ext_bin and ext != ext_json and ext != ext_scale) {
          continue;
        }
        path.replace_extension(use_ext);
        auto &ok = path_ok[path];
        ok = ok or ext == use_ext;
      }
      for (auto &[path, ok] : path_ok) {
        if (not ok) {
          fmt::println(
              "{}:{} warning: {} is missing, "
              "but files with other extensions are available",
              __FILE__,
              __LINE__,
              path.native());
          continue;
        }
        paths.emplace(path);
      }
    }

    // auto decode(qtils::BytesIn raw) const {
    //   return jam::decode<T>(raw, config);
    // }
    //
    // auto encode(const auto &value) const {
    //   return jam::encode(value, config);
    // }
    //
    // static auto readRaw(const std::filesystem::path &path) {
    //   return qtils::readBytes(path);
    // }
  };

  template <typename T>
  struct TestT : testing::TestWithParam<
                     std::pair<std::shared_ptr<T>, std::filesystem::path>> {
    const T &vectors = *this->GetParam().first;
    const std::filesystem::path &path = this->GetParam().second;
  };
}  // namespace jam::test_vectors
