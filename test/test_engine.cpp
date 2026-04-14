#include "fmt/base.h"
#include "search/lib.h"
#include "search/policies.h"
#include "gtest/gtest.h"
#include <cstdio>
#include <fstream>
static std::mutex sc_mtx;

static std::string writeTempFile(const std::string &content) {

  std::string filename =
      "/tmp/engine_test_" + std::to_string(std::rand() % 1000000) + ".dat";

  std::ofstream ofs(filename, std::ios::binary);
  ofs.write(content.data(), content.size());
  ofs.close();

  return std::string(filename);
}

TEST(Engine, simpleCharMatch) {
  std::scoped_lock<std::mutex> lock(sc_mtx);
  InputConfig::resetChunkSize();
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
  std::scoped_lock<std::mutex> lock(sc_mtx);
  InputConfig::resetChunkSize();
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
TEST(Engine, LockedPolicySimpleMatching) {
  std::scoped_lock<std::mutex> lock(sc_mtx);
  InputConfig::resetChunkSize();
  Re2Matcher matcher("abcdef");
  auto policy = std::make_unique<LockedPolicy>(matcher);
  Engine<LockedPolicy> engine(std::move(policy));

  std::string input = "xxxxabcdefxxxx";
  std::string filePath = writeTempFile(input);

  engine.setFilePath(filePath);
  int result = engine.run();
  EXPECT_EQ(result, 1);

  std::remove(filePath.c_str());
}
TEST(Engine, LockedPolicyBoundaryMatching) {
  std::scoped_lock<std::mutex> lock(sc_mtx);
  InputConfig::resetChunkSize();
  InputConfig::setChunkSize(16); // small chunk for testing
  InputConfig::init("abcdef");

  Re2Matcher matcher("abcdef");
  auto policy = std::make_unique<LockedPolicy>(matcher);
  Engine<LockedPolicy> engine(std::move(policy));

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
