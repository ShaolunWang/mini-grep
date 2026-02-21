#pragma once
#include "queue/queue.h"
#include "search/policies.h"
#include "gtest/gtest.h"
#include <cstring>

TEST(SequentialPolicyTest, SimpleCount) {
  Re2Matcher matcher("abc"); // however you construct it
  SequentialPolicy policy(matcher);
  constexpr std::string_view text = "abc abc abc";
  auto buffer = std::make_unique<char[]>(text.size());
  std::memcpy(buffer.get(), text.data(), text.size());

  Job job(std::move(buffer), text.size());

  policy.submit(std::move(job));
  policy.finish();

  EXPECT_EQ(policy.wait(), 3);
}
