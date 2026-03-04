#include "search/lib.h"
#include "fmt/base.h"
template <ExecutorPolicy Policy> void Engine<Policy>::read_io() {
  const size_t chunk_size = InputConfig::getChunkSize();
  std::vector<char> buffer(chunk_size);
  while (true) {
    auto buffer = std::make_unique<char[]>(chunk_size);
    ssize_t n = ::read(STDIN_FILENO, buffer.get(), chunk_size);

    if (n < 0) {
      fmt::report_error("uh, what");
      break;
    }
    if (n == 0) {
      break;
    };
    m_policy.submit(Job(std::move(buffer), static_cast<std::size_t>(n)));
  }
}
