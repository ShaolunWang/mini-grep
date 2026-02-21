#pragma once
#include "matcher.h"
#include <atomic>
#include <utility>
#include <vector>

struct Job {
public:
  Job(const Job &) = delete;
  Job &operator=(const Job &) = delete;
  Job(Job &&job) noexcept : buffer{std::move(job.buffer)}, size{job.size} {};
  Job &operator=(Job &&) noexcept = default;
  Job(std::unique_ptr<char[]> buffer, std::size_t size) noexcept
      : buffer(std::move(buffer)), size{size} {}
  std::unique_ptr<char[]> buffer;
  std::size_t size;
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
    static_cast<Derived &>(*this)->submit(std::forward<std::vector<Job>>(jobs));
  }
  friend Derived;
};

struct SequentialPolicy : public ExecutorPolicyBased<SequentialPolicy> {
public:
  explicit SequentialPolicy(Re2Matcher &m_matcher) : m_matcher{m_matcher} {}
  void submit(Job job) {
    std::string_view chunk(job.buffer.get(), job.size);
    m_total += m_matcher.match(chunk);
  }
  void finish() {}
  std::size_t wait() const { return m_total; }

private:
  Re2Matcher &m_matcher;
  std::size_t m_total{0};
};

struct LockedPolicy : public ExecutorPolicyBased<LockedPolicy> {
public:
  explicit LockedPolicy(Re2Matcher &m_matcher) : m_matcher(m_matcher) {}

private:
  Re2Matcher &m_matcher;
  std::atomic<size_t> m_total{0};
};
