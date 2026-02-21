#pragma once
#include "matcher.h"
#include <atomic>
#include <string>
#include <thread>
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
  const std::string match;
};

/**
 * @brief Defining executor strategy
 * @tparam Executor
 */
template <typename Executor>
concept ExecutorPolicy = requires(Executor policy, Job job) {
  { policy.submit(job) } -> std::same_as<void>;
  // TODO(me): { policy.finish() } -> std::same_as<void>;
};

/**
 * @brief crtp
 * @tparam Derived
 * @param job
 */
template <typename Derived> struct ExecutorPolicyBased {
private:
  ExecutorPolicyBased() = default;

public:
  template <typename Job> void process_chunks(const std::vector<Job> &&jobs) {
    static_cast<Derived &>(*this)->process_chunk_impl(
        std::forward<std::vector<Job>>(jobs));
  }
  friend Derived;
};

struct SequentialPolicy : public ExecutorPolicyBased<SequentialPolicy> {
public:
  explicit SequentialPolicy(Re2Matcher &m_matcher) : m_matcher(m_matcher) {}
  template <typename Job>
  void process_chunk_impl(const std::vector<Job> &jobs) {
    for (const auto &job : jobs) {
      std::size_t local_count = 0;
      const char *data = job.file->data();
      std::size_t begin = job.offset;
      std::size_t end = job.offset + job.length;
      std::string_view chunk(data + begin, end - begin);
      m_total += m_matcher.match(chunk);
    }
  }
  std::size_t total() const { return m_total; }

private:
  Re2Matcher &m_matcher;
  std::size_t m_total{0};
};

// TODO(me):  impl
struct LockedPolicy : public ExecutorPolicyBased<LockedPolicy> {
public:
  explicit LockedPolicy(Re2Matcher &m_matcher) : m_matcher(m_matcher) {}

private:
  Re2Matcher &m_matcher;
  std::atomic<size_t> m_total{0};
};
