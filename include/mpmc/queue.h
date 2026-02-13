#include <cstddef>
#include <thread>
#include <vector>
template <typename T> class Queue {
public:
  explicit Queue(size_t capacity) {}

  void push(const T &v);
  bool try_push(const T &v);

  void pop(T &out);
  bool try_pop(const T &v);

private:
  struct Slot {
    T value;
    bool full = false;
  };
  std::vector<Slot> m_slots;
  size_t m_head = 0;
  size_t m_tail = 0;
  std::mutex m_mtx;
};

template <typename T> bool Queue<T>::try_push(const T &v) {
  std::scoped_lock<std::mutex> lock(m_mtx);
  // checking whether we are wrapped around
  // we could be at the end and head points to the end + 1
  Slot &s = m_slots[m_head % m_slots.size()];
  // if it's occupied, we bail
  if (s.full) {
    return false;
  }

  // placement
  s.value = v;
  s.full = true;
  m_head++;
  return true;
}

template <typename T> void Queue<T>::push(const T &v) {
  while (!try_push(v)) {
    // gives up the currect timeslice and re-insert it into the queue
    std::this_thread::yield();
  }
}
template <typename T> void Queue<T>::pop(T &out) {}
template <typename T> bool Queue<T>::try_pop(const T &v) {}
