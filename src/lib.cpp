#include "search/lib.h"
#include "fmt/base.h"
#include <fstream>
#include <span>
#include <string>
template class Engine<SequentialPolicy>;
template class Engine<LockedPolicy>;
template class Engine<LockFreeSPSCPolicy>;
template <ExecutorPolicy Policy> void Engine<Policy>::read_io() {

  const size_t chunk_size = InputConfig::getChunkSize();
  const size_t pattern_size = InputConfig::getPattern().size();

  std::unique_ptr<char[]> tail_buffer{};
  size_t tail_size = 0;

  std::ifstream file(m_filePath, std::ios::binary);
  if (!file) {
    fmt::println("Failed to open file: {}", m_filePath);
    return;
  }
  while (file) {
    auto buffer = std::make_unique<char[]>(chunk_size);
    file.read(buffer.get(), chunk_size);
    auto read_size = static_cast<size_t>(file.gcount());
    if (read_size == 0) {
      break;
    }

    // create new tail before we submit
    std::unique_ptr<char[]> new_tail;
    auto new_tail_size =
        (pattern_size > 1) ? std::min(read_size, pattern_size - 1) : 0;
    if (new_tail_size > 0) {
      new_tail = std::make_unique<char[]>(new_tail_size);
      std::ranges::copy(
          std::span(buffer.get() + read_size - new_tail_size, new_tail_size),
          new_tail.get());
    } else {
      new_tail.reset();
    }

    if (tail_size > 0) {
      auto combined =
          std::make_unique_for_overwrite<char[]>(tail_size + read_size);
      std::ranges::copy(std::span(tail_buffer.get(), tail_size),
                        combined.get());
      std::ranges::copy(std::span(buffer.get(), read_size),
                        combined.get() + tail_size);
      m_policy->submit(Job{std::move(combined), tail_size + read_size});
    } else {
      m_policy->submit(Job{std::move(buffer), read_size});
    }
    tail_buffer = std::move(new_tail);
    tail_size = new_tail_size;
  }
}
