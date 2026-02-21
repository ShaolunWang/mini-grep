#pragma once

#include "fmt/base.h"
#include "policies.h"
#include <cstddef>
#include <unistd.h>
#include <vector>

static constexpr std::size_t CHUNK_SIZE = 1 << 20; // 1 MB
                                                   //

/**
 * @brief split files
 *
 * @tparam Executor must be able to submit and finish
 * @param file path with buffer
 * @param executor Executor type
 */
template <typename ExecutorPolicy> void read_io(ExecutorPolicy &executor) {

  std::vector<char> buffer(CHUNK_SIZE);
  while (true) {
    auto buffer = std::make_unique<char[]>(CHUNK_SIZE);
    ssize_t n = ::read(STDIN_FILENO, buffer.get(), CHUNK_SIZE);

    if (n <= 0) {
      fmt::report_error("uh, whar");
    }
    executor.submit(
        Job{.buffer = std::move(buffer), .size = static_cast<std::size_t>(n)});
  }

}
