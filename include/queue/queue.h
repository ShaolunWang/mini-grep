#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <vector>
template <typename T> class LockedQueue {
public:
  explicit LockedQueue(size_t capacity) { m_slots.resize(capacity); }

  void emplace(const T &v);
  T pop();

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

/**
 *
 *  1 -> 2 (tail)-> 3 -> 4 -> 5(head)
 *
 **/

template <typename T> void LockedQueue<T>::emplace(const T &v) {
  std::unique_lock<std::mutex> lock{m_mtx};
  cv.wait(lock, [&] { return m_head - m_tail < m_slots.size(); });
  auto &s = m_slots[m_head % m_slots.size()];

  // until it's not full!
  cv.wait(lock, [&] { return !s.full; });
  s.value = v;
  s.full = true;
  m_head++;
  lock.unlock();
  cv.notify_all();
}

template <typename T> T LockedQueue<T>::pop() {
  std::unique_lock<std::mutex> lock(m_mtx);
  cv.wait(lock, [&] { return m_head > m_tail; });
  auto &s = m_slots[m_tail % m_slots.size()];

  Slot v = std::move(s);
  s.full = false;
  m_tail++;
  lock.unlock();
  cv.notify_all();
  return v.value;
}
