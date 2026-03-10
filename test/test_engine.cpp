#include "search/lib.h"
#include "search/policies.h"
#include "gtest/gtest.h"

TEST(Engine, simpleCharMatch) {
  Re2Matcher matcher("abcdef");
  auto policy = std::make_unique<LockFreeSPSCPolicy>(matcher);
  Engine<LockFreeSPSCPolicy> engine(std::move(policy));
  int pipefd[2];
  pipe(pipefd);

  std::string input = "xxxxabcdexxxx";

  write(pipefd[1], input.data(), input.size());
  close(pipefd[1]);

  dup2(pipefd[0], STDIN_FILENO);
  engine.run();
}

TEST(Engine, EngineBoundary) {
  InputConfig::setChunkSize(16); // small chunk for testing
  InputConfig::init("abcdef");

  Re2Matcher matcher("abcdef");
  auto policy = std::make_unique<LockFreeSPSCPolicy>(matcher);
  Engine<LockFreeSPSCPolicy> engine(std::move(policy));

  std::string chunk1(InputConfig::getChunkSize(), 'x');
  chunk1.replace(InputConfig::getChunkSize() - 4, 4, "abcd");

  std::string chunk2(InputConfig::getChunkSize(), 'x');
  chunk2.replace(0, 2, "ef");

  int pipefd[2];
  pipe(pipefd);

  std::string input = chunk1 + chunk2;
  write(pipefd[1], input.data(), input.size());
  close(pipefd[1]);

  dup2(pipefd[0], STDIN_FILENO);

  auto result = engine.run();
  EXPECT_EQ(result, 1);
  InputConfig::resetChunkSize();
}
