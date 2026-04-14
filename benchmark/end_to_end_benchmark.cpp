#include <benchmark/benchmark.h>
#include <string>
#include <vector>

#include "search/lib.h"
#include "search/policies.h"

template <typename Policy> static void BM_Engine_File(benchmark::State &state) {

  const size_t chunk_size = state.range(0);

  for (auto _ : state) {
    state.PauseTiming();

    InputConfig::setChunkSize(chunk_size);
    InputConfig::init("abcdef");

    Re2Matcher matcher("abcdef");
    auto policy = std::make_unique<Policy>(matcher);
    Engine<Policy> engine(std::move(policy));

    engine.setFilePath(GEN_INPUT_FILE);

    state.ResumeTiming();
    auto x = engine.run();
    benchmark::DoNotOptimize(x);
  }

  InputConfig::resetChunkSize();
}

#define REGISTER_POLICY(policy, name)                                          \
  BENCHMARK_TEMPLATE(BM_Engine_File, policy)                                   \
      ->Args({4096})                                                           \
      ->Args({65536})                                                          \
      ->Args({65536})                                                          \
      ->Args({65536})                                                          \
      ->Name(name)                                                             \
      ->MinTime(0.05)                                                          \
      ->Iterations(10000);

REGISTER_POLICY(LockFreeSPSCPolicy, "SPSC")
REGISTER_POLICY(LockedPolicy, "Mutex")
REGISTER_POLICY(SequentialPolicy, "SingleThread")

BENCHMARK_MAIN();
