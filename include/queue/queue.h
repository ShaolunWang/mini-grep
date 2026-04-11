
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
      : m_capacity(capacity), m_storage(static_cast<std::byte *>(
                                  ::operator new(sizeof(T) * capacity))) {}

  ~LockedQueue() {
    close();
    while (m_head > m_tail) {
      destroy_front();
    }

    ::operator delete(m_storage);
  }

  LockedQueue(const LockedQueue &) = delete;
  LockedQueue &operator=(const LockedQueue &) = delete;

  void close() {
    std::lock_guard lock(m_mtx);
    m_closed = true;
    m_cv.notify_all();
  }

  template <typename U> bool emplace(U &&v) {
    std::unique_lock lock(m_mtx);

    m_cv.wait(lock, [&] { return m_closed || (m_head - m_tail) < m_capacity; });

    if (m_closed)
      return false;

    size_t idx = m_head % m_capacity;
    T *place = launder_ptr(idx);

    new (place) T(std::forward<U>(v));

    ++m_head;

    lock.unlock();
    m_cv.notify_one();
    return true;
  }

  std::optional<T> pop() {
    std::unique_lock lock(m_mtx);

    m_cv.wait(lock, [&] { return m_closed || m_head > m_tail; });

    if (m_head == m_tail) {
      return std::nullopt;
    }

    size_t idx = m_tail % m_capacity;
    T *place = launder_ptr(idx);

    T out = std::move(*place);
    std::destroy_at(place);

    ++m_tail;

    lock.unlock();
    m_cv.notify_one();
    return out;
  }

private:
  void destroy_front() {
    size_t idx = m_tail % m_capacity;
    T *place = launder_ptr(idx);

    std::destroy_at(place);
    ++m_tail;
  }

  // launders the pointer
  // so that we can propery work on that
  T *launder_ptr(size_t idx) const noexcept {
    return std::launder(reinterpret_cast<T *>(m_storage + (idx * sizeof(T))));
  }

  size_t m_capacity;
  std::byte *m_storage;

  size_t m_head{0};
  size_t m_tail{0};

  bool m_closed{false};

  std::mutex m_mtx;
  std::condition_variable m_cv;
};
