#include "ringbuffer/ringbuffer.h"
#include "gtest/gtest.h"
#include <atomic>
#include <thread>

using namespace std::chrono_literals;

TEST(LockfreeSPSCRingBufferTest, StartsEmpty) {
  LockfreeSPSCRingBuffer<int> q(8);

  int value;
  EXPECT_FALSE(q.pop(value));
}

TEST(LockfreeSPSCRingBufferTest, PushPopSingleElement) {
  LockfreeSPSCRingBuffer<int> q(8);

  EXPECT_TRUE(q.push(42));

  int out = 0;
  EXPECT_TRUE(q.pop(out));
  EXPECT_EQ(out, 42);

  EXPECT_FALSE(q.pop(out));
}

TEST(LockfreeSPSCRingBufferTest, FillUntilFull) {
  constexpr size_t capacity = 4;
  LockfreeSPSCRingBuffer<int> q(capacity);

  size_t pushed = 0;
  while (q.push(static_cast<int>(pushed))) {
    pushed++;
  }

  EXPECT_FALSE(q.push(999));
}

TEST(LockfreeSPSCRingBufferTest, WrapAround) {
  constexpr size_t capacity = 4;
  LockfreeSPSCRingBuffer<int> q(capacity);

  EXPECT_TRUE(q.push(1));
  EXPECT_TRUE(q.push(2));
  EXPECT_TRUE(q.push(3));

  int out;

  EXPECT_TRUE(q.pop(out));
  EXPECT_EQ(out, 1);

  EXPECT_TRUE(q.pop(out));
  EXPECT_EQ(out, 2);

  EXPECT_TRUE(q.push(4));
  EXPECT_TRUE(q.push(5));

  EXPECT_TRUE(q.pop(out));
  EXPECT_EQ(out, 3);

  EXPECT_TRUE(q.pop(out));
  EXPECT_EQ(out, 4);

  EXPECT_TRUE(q.pop(out));
  EXPECT_EQ(out, 5);

  EXPECT_FALSE(q.pop(out));
}

// Threaded SPSC test

TEST(LockfreeSPSCRingBufferTest, ThreadedSPSCIntegrity) {
  constexpr size_t capacity = 1024;
  constexpr size_t iterations = 1'000'000;

  LockfreeSPSCRingBuffer<size_t> q(capacity);

  std::vector<size_t> consumed;
  consumed.reserve(iterations);

  std::jthread producer([&]() {
    for (size_t i = 0; i < iterations; ++i) {
      while (!q.push(i)) {
        std::this_thread::yield();
      }
    }
  });

  std::jthread consumer([&]() {
    size_t value;
    size_t expected = 0;

    // using counters to make sure that
    // we are actually consuming
    while (expected < iterations) {
      if (q.pop(value)) {
        ASSERT_EQ(value, expected);
        consumed.push_back(value);
        ++expected;
      } else {
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();

  ASSERT_EQ(consumed.size(), iterations);
}

TEST(LockfreeSPSCRingBufferTest, ThreadedSmallCapacityWrap) {
  constexpr size_t capacity = 8;
  constexpr size_t iterations = 200000;

  LockfreeSPSCRingBuffer<int> q(capacity);

  std::atomic<bool> producer_done{false};
  std::atomic<int> last_seen{-1};

  std::jthread producer([&]() {
    for (int i = 0; i < static_cast<int>(iterations); ++i) {
      while (!q.push(i)) {
        std::this_thread::yield();
      }
    }
    producer_done.store(true, std::memory_order_release);
  });

  std::jthread consumer([&]() {
    int value;
    int expected = 0;

    while (!producer_done.load(std::memory_order_acquire) ||
           expected < static_cast<int>(iterations)) {

      if (q.pop(value)) {
        EXPECT_EQ(value, expected);
        ++expected;
        last_seen.store(value, std::memory_order_relaxed);
      } else {
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(last_seen.load(), static_cast<int>(iterations - 1));
}
