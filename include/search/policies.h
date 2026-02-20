#pragma once
#include <string>
#include <utility>
#include <vector>
struct FileMetadata {
  std::string path;
  std::vector<char> buffer; // chunk
};
struct Job {
  const FileMetadata *file;
  std::size_t offset;
  std::size_t length; // length of the chunk in bytes
};

/**
 * @brief Defining executor strategy
 * @tparam Executor
 */
template <typename Executor>
concept ExecutorPolicy = requires(Executor policy, Job job) {
  { policy.submit(job) } -> std::same_as<void>;
  // TODO: { policy.finish() } -> std::same_as<void>;
};

/**
 * @brief crtp
 * @tparam Derived
 * @param job
 */
template <typename Derived> struct ExecutorPolicyBased {
  template <typename Job> void process_chunks(const Job &&job) {
    static_cast<Derived>(*this)->process_chunk_impl(std::forward<Job>(job));
  }
};

struct SequentialPolicy : public ExecutorPolicyBased<SequentialPolicy> {
  template <typename Job> void process_chunk_impl(const Job &job) {
    std::size_t local_count = 0;
    const char *data = job.file->data();
    std::size_t begin = job.offset;
    std::size_t end = job.offset + job.length;
    // TODO: pattern matching
  }
};
