#pragma once
#include <string>
#include <vector>

static constexpr std::size_t CHUNK_SIZE = 1 << 20; // 1 MB
                                                   //
struct FileMetadata {
  std::string path;
  std::vector<char> buffer; // entire file contents
};

struct Job {
  const FileMetadata *file;
  std::size_t offset;
  std::size_t length; // length of the chunk in bytes
};

struct Chunk {
  const FileMetadata *file;
  std::uint16_t offset;
  std::uint16_t length;
};

/**
 * @brief Defining executor strategy
 * @tparam Executor
 */
template <typename Executor>
concept ExecutorPolicy = requires(Executor policy, Job job) {
  { policy.submit(job) } -> std::same_as<void>;
  // { policy.finish() } -> std::same_as<void>;
};

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
    executor.submit(Job{file, offset, len});
  }
}
