/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/history/vectors.hpp>

GTEST_VECTORS(History, history);

/**
 * Check python generated scale encoding/decoding against test vectors.
 */
//GTEST_VECTORS_TEST_REENCODE(History, history);

class HistoryTest_Reencode_Test
    : public HistoryTest,
      private ::testing::internal::GTestNonCopyable {
 public:
  HistoryTest_Reencode_Test() {}
  void TestBody() override;

 private:
  static int AddToRegistry() {
    ::testing::UnitTest::GetInstance()
        ->parameterized_test_registry()
        .GetTestSuitePatternHolder<HistoryTest>(
            "HistoryTest", ::testing::internal::CodeLocation("_file_name_", 16))
        ->AddTestPattern("HistoryTest",
                         "Reencode",
                         new ::testing::internal::TestMetaFactory<
                             HistoryTest_Reencode_Test>(),
                         ::testing::internal::CodeLocation("_file_name_", 16));
    return 0;
  }
  __attribute__((__unused__)) static int gtest_registering_dummy_;
};
int HistoryTest_Reencode_Test::gtest_registering_dummy_ =
    HistoryTest_Reencode_Test::AddToRegistry();
void HistoryTest_Reencode_Test::TestBody() {
  using jam::test_vectors::getTestLabel;
  fmt::println("Test reencode for '{}'\n", getTestLabel(path));
  auto expected = vectors.readRaw(path);
  std::optional<jam::test_vectors::history::TestCase> decoded_;
  try {
    decoded_ = vectors.decode(expected);
  } catch (const boost::wrapexcept<std::system_error> &e) {
    return ::testing::internal::AssertHelper(
               ::testing::TestPartResult::kFatalFailure,
               "_file_name_",
               16,
               "Failed") = ::testing::Message()
                        << "Can't decode input file: " << e.what();
  } catch (const std::exception &e) {
    return ::testing::internal::AssertHelper(
               ::testing::TestPartResult::kFatalFailure,
               "_file_name_",
               16,
               "Failed") = ::testing::Message()
                        << "Can't decode input file: " << e.what();
  }
  auto &decoded = decoded_.value();
  auto encoded_ = vectors.encode(decoded);
  auto &reencoded = encoded_;
  switch (0)
  case 0:
  default:
    if (const ::testing::AssertionResult gtest_ar =
            (::testing::internal::EqHelper::Compare(
                "reencoded", "expected", reencoded, expected)))
      ;
    else::testing::internal::AssertHelper(
          ::testing::TestPartResult::kNonFatalFailure,
          "_file_name_",
          16,
          gtest_ar.failure_message()) = ::testing::Message();};
