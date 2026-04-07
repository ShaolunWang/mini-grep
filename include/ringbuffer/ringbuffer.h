#pragma once
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>
#include <optional>
template <typename T> class LockfreeSPSCRingBuffer {
public:
  explicit LockfreeSPSCRingBuffer(size_t capacity) : m_capacity{capacity} {
    m_container = static_cast<Element *>(malloc(capacity * sizeof(Element)));
  }
  // no more cast littering
  bool push(const T &v)
    requires std::copyable<T>
  {
    // tail is only modified by producer
    // so we don't care about the ordering as long as it's own ordering
    // is ensured
    const size_t currentTail = m_tail.load(std::memory_order_relaxed);

    // head can be modified by the consumer, so load acquire is required
    if ((currentTail + 1) % m_capacity ==
        m_head.load(std::memory_order_acquire))
      return false;
    new (&m_container[currentTail].storage) T(v);
    // storing so the next tail read is ok
    m_tail.store((currentTail + 1) % m_capacity, std::memory_order_release);
    return true;
  }
  bool emplace(T &&v) {
    // when we emplace it, we wanna check whether tail is catching up with the
    // head
    const std::size_t currentTail = m_tail.load(std::memory_order_relaxed);

    // full
    if ((currentTail + 1) % m_capacity ==
        m_head.load(std::memory_order_acquire)) {
      return false;
    }
    auto *ptr = std::bit_cast<T *>(&m_container[currentTail].storage);
    new (&m_container[currentTail].storage) T(std::move(v));
    m_tail.store((currentTail + 1) % m_capacity, std::memory_order_release);
    return true;
  };

  std::optional<T> pop() {
    const size_t currentHead = m_head.load(std::memory_order_relaxed);
    if (currentHead == m_tail.load(std::memory_order_acquire)) {
      return std::nullopt;
    };
    auto *ptr = std::bit_cast<T *>(&m_container[currentHead].storage);
    std::optional<T> out = std::move(*ptr);
    std::destroy_at(ptr);
    m_head.store((currentHead + 1) % m_capacity, std::memory_order_release);
    return out;
  };
  ~LockfreeSPSCRingBuffer() {
    while (pop()) {
    }
    std::free(m_container);
  }
  bool empty() const {
    return m_head.load(std::memory_order_acquire) ==
           m_tail.load(std::memory_order_acquire);
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
