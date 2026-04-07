#include "search/lib.h"
#include "search/policies.h"
#include "gtest/gtest.h"
#include <cstdio>
#include <fstream>

static std::string writeTempFile(const std::string &content) {

  srand(time(nullptr));
  auto stamp = rand() % 1357;
  std::string tmp_name = "/tmp/engine_test_" + std::to_string(stamp);
  int fd = mkstemp(tmp_name.data());
  if (fd < 0)
    throw std::runtime_error("Failed to create temp file");

  std::ofstream ofs(tmp_name, std::ios::binary);
  ofs.write(content.data(), content.size());
  ofs.close();
  close(fd);

  return std::string(tmp_name);
}

TEST(Engine, simpleCharMatch) {
  Re2Matcher matcher("abcdef");
  auto policy = std::make_unique<LockFreeSPSCPolicy>(matcher);
  Engine<LockFreeSPSCPolicy> engine(std::move(policy));

  std::string input = "xxxxabcdexxxx";
  std::string filePath = writeTempFile(input);

  engine.setFilePath(filePath);
  int result = engine.run();
  EXPECT_EQ(result, 0);

  std::remove(filePath.c_str());
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

  std::string input = chunk1 + chunk2;
  std::string filePath = writeTempFile(input);

  engine.setFilePath(filePath);
  auto result = engine.run();
  EXPECT_EQ(result, 1);

  std::remove(filePath.c_str());
  InputConfig::resetChunkSize();
}
