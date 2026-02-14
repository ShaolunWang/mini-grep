#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <vector>
template <typename T> class Queue {
public:
  explicit Queue(size_t capacity) { m_slots.resize(capacity); }

  void push(const T &v);
  bool try_push(const T &v);

  T pop();
  bool try_pop();

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

template <typename T> bool Queue<T>::try_push(const T &v) {}

template <typename T> void Queue<T>::push(const T &v) {
  // NOTE: for future lock free impl:
  // https://old.reddit.com/r/cpp_questions/comments/15d938p/smart_spin_wait/l8h51ds/
  //
  // while (!try_push(v)) {
  //   // gives up the currect timeslice and re-insert it into the queue
  //   //
  //   // can do with `std::this_thread::yield();`
  //   //
  // }
  std::unique_lock<std::mutex> lock{m_mtx};
  cv.wait(lock, [&] { return m_head - m_tail < m_slots.size(); });
  auto &s = m_slots[m_head % m_slots.size()];

  // until it's not full!
  cv.wait(lock, [&] { return !s.full; });
  s.value = v;
  s.full = true;

  // NOTE: both head and tail are never decrementing
  //       so that it will always wrap correctly
  m_head++;
  lock.unlock();
  cv.notify_all();
}

template <typename T> T Queue<T>::pop() {
  std::unique_lock<std::mutex> lock(m_mtx);
  cv.wait(lock, [&] { return m_head > m_tail; });
  auto &s = m_slots[m_tail % m_slots.size()];

  Slot v = std::move(s);
  s.full = false;
  // NOTE: both head and tail are never decrementing
  //       so that it will always wrap correctly
  m_tail++;
  lock.unlock();
  cv.notify_all();
  return v.value;
}
template <typename T> bool Queue<T>::try_pop() {}
