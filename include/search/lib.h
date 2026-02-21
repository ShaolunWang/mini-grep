#pragma once

#include "policies.h"
#include <cstddef>

static constexpr std::size_t CHUNK_SIZE = 1 << 20; // 1 MB
                                                   //

/**
 * @brief split files
 *
 * @tparam Executor must be able to submit and finish
 * @param file path with buffer
 * @param executor Executor type
 */
template <typename ExecutorPolicy>
void split_files(const FileMetadata *file, ExecutorPolicy &executor) {
  std::size_t size = file->buffer.size();
  for (std::size_t offset = 0; offset < size; offset += CHUNK_SIZE) {
    // could be smaller than a chunk size
    std::size_t len = std::min(CHUNK_SIZE, size - offset);
    executor.submit(Job{.file = file, .offset = offset, .length = len});
  }
}
