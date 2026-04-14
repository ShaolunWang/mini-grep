#include "search/input.h"
#include "search/policies.h"
#include "gtest/gtest.h"
#include <cstring>

TEST(SequentialPolicyTest, NoMatches) {
  Re2Matcher matcher("xyz");
  SequentialPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc abc"));
  policy.finish();
  EXPECT_EQ(policy.wait(), 0);
}

TEST(SequentialPolicyTest, SingleJob) {
  Re2Matcher matcher("abc");
  SequentialPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc abc"));
  policy.finish();
  EXPECT_EQ(policy.wait(), 3);
}

TEST(SequentialPolicyTest, MultipleJobs) {
  Re2Matcher matcher("abc");
  SequentialPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc"));
  policy.submit(Job::make_job("abc"));
  policy.submit(Job::make_job("no match here"));
  policy.finish();
  EXPECT_EQ(policy.wait(), 3);
}

TEST(SequentialPolicyTest, EmptyChunk) {
  Re2Matcher matcher("abc");
  SequentialPolicy policy(matcher);
  policy.submit(Job::make_job(""));
  policy.finish();
  EXPECT_EQ(policy.wait(), 0);
}

TEST(SequentialPolicyTest, SimpleCount) {
  Re2Matcher matcher("abc");
  SequentialPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc abc"));
  policy.finish();
  EXPECT_EQ(policy.wait(), 3);
}

namespace {
class LockFreeSPSCPolicyTest : public ::testing::Test {
protected:
  void SetUp() override { InputConfig::init("abc"); }
};

TEST_F(LockFreeSPSCPolicyTest, NoMatches) {
  Re2Matcher matcher("xyz");
  LockFreeSPSCPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc abc"));
  EXPECT_EQ(policy.wait(), 0);
}

TEST_F(LockFreeSPSCPolicyTest, SingleJob) {
  Re2Matcher matcher("abc");
  LockFreeSPSCPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc abc"));
  EXPECT_EQ(policy.wait(), 3);
}

TEST_F(LockFreeSPSCPolicyTest, MultipleJobs) {
  Re2Matcher matcher("abc");
  LockFreeSPSCPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc"));
  policy.submit(Job::make_job("abc"));
  policy.submit(Job::make_job("no match here"));
  EXPECT_EQ(policy.wait(), 3);
}

TEST_F(LockFreeSPSCPolicyTest, EmptyChunk) {
  Re2Matcher matcher("abc");
  LockFreeSPSCPolicy policy(matcher);
  policy.submit(Job::make_job(""));
  EXPECT_EQ(policy.wait(), 0);
}

TEST_F(LockFreeSPSCPolicyTest, ManyJobs) {
  Re2Matcher matcher("abc");
  LockFreeSPSCPolicy policy(matcher);
  constexpr int N = 10'000;
  for (int i = 0; i < N; i++)
    policy.submit(Job::make_job("abc"));
  EXPECT_EQ(policy.wait(), N);
}

TEST_F(LockFreeSPSCPolicyTest, LargeInputStress) {
  Re2Matcher matcher("abc");
  LockFreeSPSCPolicy policy(matcher);

  constexpr int total_chunks = 5;
  for (int i = 0; i < total_chunks; i++) {
    std::string chunk(InputConfig::getChunkSize(), 'x');
    if (i % 2 == 0) {
      chunk.replace(54321, 3, "abc");
    }
    policy.submit(Job::make_job(chunk));
  }

  EXPECT_EQ(policy.wait(), 3);
}
TEST(LockedPolicyTest, NoMatches) {
  Re2Matcher matcher("xyz");
  LockedPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc abc"));
  EXPECT_EQ(policy.wait(), 0);
}

TEST(LockedPolicyTest, SingleJob) {
  Re2Matcher matcher("abc");
  LockedPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc abc"));
  EXPECT_EQ(policy.wait(), 3);
}

TEST(LockedPolicyTest, MultipleJobs) {
  Re2Matcher matcher("abc");
  LockedPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc"));
  policy.submit(Job::make_job("abc"));
  policy.submit(Job::make_job("no match here"));
  EXPECT_EQ(policy.wait(), 3);
}

TEST(LockedPolicyTest, EmptyChunk) {
  Re2Matcher matcher("abc");
  LockedPolicy policy(matcher);
  policy.submit(Job::make_job(""));
  policy.finish();
  EXPECT_EQ(policy.wait(), 0);
}

TEST(LockedPolicyTest, SimpleCount) {
  Re2Matcher matcher("abc");
  LockedPolicy policy(matcher);
  policy.submit(Job::make_job("abc abc abc"));
  policy.finish();
  EXPECT_EQ(policy.wait(), 3);
}
} // namespace
