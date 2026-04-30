
#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <new>
#include <optional>
#include <utility>

template <typename T> class LockedQueue {
public:
  explicit LockedQueue(size_t capacity)
      : m_capacity(capacity),
        m_storage(static_cast<T *>(malloc(sizeof(T) * capacity))) {}

  ~LockedQueue() {
    close();
    while (m_head > m_tail) {
      destroy_front();
    }

    ::operator delete(m_storage);
  }

  LockedQueue(const LockedQueue &) = delete;
  LockedQueue &operator=(const LockedQueue &) = delete;

  /**
   * @brief close the queue
   */
  void close() {
    std::lock_guard lock(m_mtx);
    m_closed = true;
    // notify all to stop everything
    m_cv.notify_all();
  }

  template <typename U> bool emplace(U &&v) {
    std::unique_lock lock(m_mtx);

    m_cv.wait(lock, [&] { return m_closed || (m_head - m_tail) < m_capacity; });

    // this is to bail out everything
    // after close is signaled
    if (m_closed)
      return false;

    size_t idx = m_head % m_capacity;
    T *place = &m_storage[idx];

    new (place) T(std::forward<U>(v));

    ++m_head;

    lock.unlock();
    m_cv.notify_one();
    return true;
  }

  std::optional<T> pop() {
    std::unique_lock lock(m_mtx);

    m_cv.wait(lock, [&] { return m_closed || m_head > m_tail; });

    // only time that things can be closed is when heads == tail
    if (m_head == m_tail && m_closed) {
      return std::nullopt;
    }

    size_t idx = m_tail % m_capacity;
    T *place = &m_storage[idx];

    std::optional<T> out = std::optional(std::move(*place));
    std::destroy_at(place);

    ++m_tail;

    lock.unlock();
    m_cv.notify_one();
    return out;
  }

private:
  void destroy_front() {
    size_t idx = m_tail % m_capacity;
    T *place = &m_storage[idx];

    std::destroy_at(place);
    ++m_tail;
  }

  // launders the pointer
  // so that we can propery work on that object

  size_t m_capacity;
  T *m_storage;

  size_t m_head{0};
  size_t m_tail{0};

  bool m_closed{false};

  std::mutex m_mtx;
  std::condition_variable m_cv;
};
