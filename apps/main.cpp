#include "fmt/format.h"
#include "search/lib.h"
#include "search/policies.h"
int main(int argc, char **argv) {

  if (argc < 2) {
    fmt::println("Usage: {} <pattern>", argv[0]);
    return 1;
  }
  const std::string pattern = argv[1];
  Re2Matcher matcher(std::move(pattern));
  auto policy = std::make_unique<LockFreeSPSCPolicy>(matcher);
  //
  Engine<LockFreeSPSCPolicy> engine(std::move(policy));
  std::string chunk1(InputConfig::getChunkSize(), 'x');

  auto x = engine.run();
  fmt::println("{}", x);
  return 0;
}
