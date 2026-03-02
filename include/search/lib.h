#pragma once

#include "fmt/base.h"
#include "input.h"
#include "policies.h"
#include <cstddef>
#include <unistd.h>
#include <vector>

/**
 * @brief split files
 *
 * @tparam Executor must be able to submit and finish
 * @param file path with buffer
 * @param executor Executor type
 */
template <typename ExecutorPolicy> void read_io(ExecutorPolicy &executor) {
  const size_t chunk_size = InputConfig::getInputConfig()->getChunkSize();
  std::vector<char> buffer(chunk_size);
  while (true) {
    auto buffer = std::make_unique<char[]>(chunk_size);
    ssize_t n = ::read(STDIN_FILENO, buffer.get(), chunk_size);

    if (n <= 0) {
      fmt::report_error("uh, what");
    }
    executor.submit(Job(std::move(buffer), static_cast<std::size_t>(n)));
  }
}

template <ExecutorPolicy Policy> class Engine {
public:
  explicit Engine(Policy policy) : m_policy{policy} {}
  size_t run() {
    split_chunks();
    launch_jobs();
  };

private:
  /**
   * @brief launch jobs on threads
   */
  void launch_jobs() {}
  void split_chunks() {
    // NOTE: naively we could just store the whole buffer in memory
    // then split the chunks later. However this might not be very good.
    // requires some testing over it
    //
    // TODO(me):
    // basically we want to split the buffer into chunks here
  };
  Policy m_policy;
};
