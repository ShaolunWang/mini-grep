#pragma once
#include "matcher.h"
#include "ringbuffer/ringbuffer.h"
#include "search/input.h"
#include <atomic>
#include <cstring>
#include <stdatomic.h>
#include <thread>
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
  static Job make_job(std::string_view text) {
    if (text.empty()) {
      auto buffer = std::make_unique<char[]>(1); // allocate at least 1 byte
      return Job(std::move(buffer), 0);
    }
    auto buffer = std::make_unique<char[]>(text.size());
    std::memcpy(buffer.get(), text.data(), text.size());
    return Job(std::move(buffer), text.size());
  }
};

/**
 * @brief Defining executor strategy
 * @tparam Executor
 */
template <typename Executor>
concept ExecutorPolicy = requires(Executor policy, Job job) {
  { policy.submit(job) } -> std::same_as<void>;
  { policy.finish() } -> std::same_as<void>;
  { policy.wait() } -> std::same_as<size_t>;
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
  template <typename Job>
  void process_chunks(this Job &self, const std::vector<Job> &&jobs) {
    self->submit(std::forward<std::vector<Job>>(jobs));
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
  void finish() {};
  std::size_t wait() { return m_total.load(memory_order_relaxed); }

private:
  Re2Matcher &m_matcher;
  std::atomic<size_t> m_total{0};
};

struct LockFreeSPSCPolicy : public ExecutorPolicyBased<LockFreeSPSCPolicy> {
public:
  // NOTE: this is weeeird :)
  explicit LockFreeSPSCPolicy(Re2Matcher &matcher)
      : m_matcher(matcher), m_queue(InputConfig::getChunkSize()) {
    m_consumer =
        std::jthread([this](const std::stop_token &stop) { consume(stop); });
  };
  void submit(Job job) {
    while (job.buffer != nullptr && job.size != 0 &&
           !m_queue.emplace(std::move(job))) {
      std::this_thread::yield();
    }
  };
  std::size_t wait() {
    m_consumer.request_stop();
    m_consumer.join();
    return m_total.load(std::memory_order_relaxed);
  };
  void finish() {};
  ~LockFreeSPSCPolicy() { m_done.store(true, std::memory_order_release); }

private:
  void consume(const std::stop_token &stop) {
    while (true) {
      if (auto job = m_queue.pop()) {
        if (job->buffer == nullptr || job->size == 0)
          continue;
        std::string_view chunk(job->buffer.get(), job->size);
        m_total.fetch_add(m_matcher.match(chunk), std::memory_order_relaxed);
      } else if (stop.stop_requested() && m_queue.empty()) {
        break;
      } else {
        std::this_thread::yield();
      }
    }
  };
  Re2Matcher &m_matcher;
  std::atomic<size_t> m_total{0};
  std::jthread m_consumer;
  std::atomic<bool> m_done{false};
  LockfreeSPSCRingBuffer<Job> m_queue; // your lock-free SPSC ring buffer
};
