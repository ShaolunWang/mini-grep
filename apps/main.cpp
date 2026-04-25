#include "fmt/format.h"
#include "search/lib.h"
#include "search/policies.h"
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fmt::println("Usage: {} <pattern>", argv[0]);
    return 1;
  }

  const std::string pattern = argv[2];
  Re2Matcher matcher(std::move(pattern));
  auto policy = std::make_unique<LockedPolicy>(matcher);
  Engine<LockedPolicy> engine(std::move(policy));

  engine.setFilePath(argv[1]);
  auto x = engine.run();
  fmt::println("{}", x);
  return 0;
}
