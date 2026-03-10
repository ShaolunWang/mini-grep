#include "search/lib.h"
#include "fmt/format.h"
#include "search/policies.h"
int main() {
  Re2Matcher matcher("abcdef");
  auto policy = std::make_unique<LockFreeSPSCPolicy>(matcher);
  //
  Engine<LockFreeSPSCPolicy> engine(std::move(policy));
  std::string chunk1(InputConfig::getChunkSize(), 'x');

  auto x = engine.run();
  fmt::println("{}", x);
  return 0;
}
