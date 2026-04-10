#pragma once
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <vector>
template <typename T> class LockedQueue {
public:
  explicit LockedQueue(size_t capacity) { m_slots.resize(capacity); }

  template <typename U> void emplace(U &&v) {
    std::unique_lock<std::mutex> lock{m_mtx};

    cv.wait(lock, [&] { return (m_head - m_tail) < m_slots.size(); });

    auto &s = m_slots[m_head % m_slots.size()];
    s.value = std::forward<U>(v);
    s.full = true;
    ++m_head;

    lock.unlock();
    cv.notify_one();
  };
  T pop() {
    std::unique_lock<std::mutex> lock(m_mtx);
    cv.wait(lock, [&] { return m_head > m_tail; });
    auto &s = m_slots[m_tail % m_slots.size()];

    T v = std::move(s.value);
    s.full = false;
    m_tail++;
    lock.unlock();
    cv.notify_all();
    return v;
  }

private:
  struct Slot {
    T value;
    bool full{false};
  };
  std::vector<Slot> m_slots;
  size_t m_head{0};
  size_t m_tail{0};
  std::mutex m_mtx;
  std::condition_variable cv;
};
