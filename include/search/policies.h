#pragma once
#include "fmt/base.h"
#include "input.h"
#include "matcher.h"
#include "queue/queue.h"
#include "ringbuffer/ringbuffer.hpp"
#include <atomic>
#include <cstring>
#include <thread>
#include <utility>

struct Job {
public:
  Job(const Job &) = delete;
  Job &operator=(const Job &) = delete;
  Job(Job &&job) noexcept : buffer{std::move(job.buffer)}, size{job.size} {
    job.size = 0;
    job.stop = false;
  };
  Job &operator=(Job &&other) noexcept {
    if (this != &other) {
      buffer = std::move(other.buffer);
      size = other.size;
      other.size = 0;
      this->stop = other.stop;
    }
    return *this;
  }
  Job(std::unique_ptr<char[]> buffer, std::size_t size) noexcept
      : buffer(std::move(buffer)), size{size} {}
  std::unique_ptr<char[]> buffer;
  std::size_t size;
  bool stop{false};
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
concept ExecutorPolicy = requires(Executor policy, Job &&job) {
  { policy.submit(std::move(job)) } -> std::same_as<void>;
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
  // template <typename Job>
  // void process_chunks(this Job &self, const std::vector<Job> &&jobs) {
  //   self->submit(std::forward<std::vector<Job>>(jobs));
  // }

  // NOTE: Thank you github ci for not supporting deducing this
  // I don't plan to get gcc14 on ci because why bother :)

  template <typename Job> void submit(Job &&job) {
    static_cast<Derived &>(*this)->submit(std::forward<Job>(job));
  }
  void finish() { static_cast<Derived &>(*this)->finish(); }
  std::size_t wait() { static_cast<Derived &>(*this)->wait(); }
  friend Derived;
};

struct SequentialPolicy : public ExecutorPolicyBased<SequentialPolicy> {
public:
  explicit SequentialPolicy(Re2Matcher &m_matcher) : m_matcher{m_matcher} {}
  void submit(Job &&job) {
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
  explicit LockedPolicy(Re2Matcher &m_matcher)
      : m_matcher(m_matcher), m_queue{InputConfig::getChunkSize()} {
    m_consumer = std::jthread([this]() { consume(); });
  }

  void submit(Job &&job) { m_queue.emplace(std::move(job)); }

  void finish() {}

  std::size_t wait() {
    m_queue.close();
    m_consumer.join();
    return m_total.load(std::memory_order_relaxed);
  }

private:
  void consume() {
    while (true) {
      auto job = m_queue.pop();
      if (!job) {
        break;
      }

      if (job->size == 0)
        continue;

      std::string_view chunk(job->buffer.get(), job->size);
      m_total.fetch_add(m_matcher.match(chunk), std::memory_order_relaxed);
    }
  }

  Re2Matcher &m_matcher;
  std::atomic<size_t> m_total{0};
  LockedQueue<Job> m_queue;
  std::jthread m_consumer;
};
;

struct LockFreeSPSCPolicy : public ExecutorPolicyBased<LockFreeSPSCPolicy> {
public:
  explicit LockFreeSPSCPolicy(Re2Matcher &matcher)
      : m_matcher(matcher), m_queue(InputConfig::getChunkSize()) {
    m_consumer =
        std::jthread([this](const std::stop_token &stop) { consume(stop); });
  };
  void submit(Job &&job) {
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
  ~LockFreeSPSCPolicy() = default;

  LockFreeSPSCPolicy(const LockFreeSPSCPolicy &) = delete;
  LockFreeSPSCPolicy &operator=(const LockFreeSPSCPolicy &) = delete;

  LockFreeSPSCPolicy(LockFreeSPSCPolicy &&) = delete;
  LockFreeSPSCPolicy &operator=(LockFreeSPSCPolicy &&) = delete;

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
  RingBuffer<Job> m_queue;
};
