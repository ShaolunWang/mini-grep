#pragma once
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>
template <typename T> class LockfreeSPSCRingBuffer {
public:
  explicit LockfreeSPSCRingBuffer(size_t capacity) : m_capacity{capacity} {
    m_container = static_cast<Element *>(malloc(capacity * sizeof(Element)));
  }
  bool push(const T &v) {
    // when we push it, we wanna check whether tail is catching up with the head
    const std::size_t currentTail = m_tail.load(std::memory_order_relaxed);

    // full
    if ((currentTail + 1) % m_capacity ==
        m_head.load(std::memory_order_acquire)) {
      return false;
    }
    auto *ptr = std::bit_cast<T *>(&m_container[currentTail].storage);
    std::construct_at(ptr, v);
    m_tail.store((currentTail + 1) % m_capacity, std::memory_order_release);
    return true;
  };

  bool pop(T &out) {
    const size_t currentHead = m_head.load(std::memory_order_relaxed);
    if (currentHead == m_tail.load(std::memory_order_acquire)) {
      return false;
    };
    auto *ptr = std::bit_cast<T *>(&m_container[currentHead].storage);
    out = std::move(*ptr);
    std::destroy_at(ptr);
    m_head.store((currentHead + 1) % m_capacity, std::memory_order_release);
    return true;
  };
  ~LockfreeSPSCRingBuffer() {
    T tmp;
    while (pop(tmp)) {
    }
    std::free(m_container);
  }

  LockfreeSPSCRingBuffer(const LockfreeSPSCRingBuffer &) = delete;
  LockfreeSPSCRingBuffer &operator=(const LockfreeSPSCRingBuffer &) = delete;

private:
// get the machine cache line size
#ifdef __cpp_lib_hardware_interference_size
  static constexpr size_t cache_line_size =
      std::hardware_destructive_interference_size;
#else
  static constexpr size_t cache_line_size = 64;
#endif
  struct Element {
    // NOTE: this construct a byte array for a single element
    alignas(T) std::array<std::byte, sizeof(T)> storage;
  };

  Element *m_container;
  size_t m_capacity;

  alignas(cache_line_size) std::atomic<size_t> m_head{0};
  alignas(cache_line_size) std::atomic<size_t> m_tail{0};
  ;
};
