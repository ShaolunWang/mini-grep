#include "queue/queue.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <mutex>
#include <thread>

TEST(QueueTest, SingleThreadPushPop) {
  LockedQueue<int> q(4);

  q.emplace(10);
  q.emplace(20);

  std::optional<int> value = q.pop();
  if (!value)
    return;
  EXPECT_EQ(value, 10);

  value = q.pop();
  EXPECT_EQ(value, 20);
}

TEST(QueueTest, MultiThreadPushPop) {
  const size_t capacity = 10;
  const size_t num_items = 1000;

  LockedQueue<int> q(capacity);

  std::vector<int> results;
  std::mutex res_mutex;

  std::thread producer([&]() {
    for (int i = 0; i < num_items; ++i) {
      q.emplace(i);
    }
  });

  // Consumer thread
  std::thread consumer([&]() {
    for (int i = 0; i < num_items; ++i) {
      std::optional<int> value = q.pop();
      if (!value)
        break;
      std::scoped_lock<std::mutex> lk(res_mutex);
      results.emplace_back(value.value());
    }
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(results.size(), num_items);
  std::ranges::sort(results);
  for (int i = 0; i < num_items; ++i) {
    EXPECT_EQ(results[i], i);
  }
}

TEST(QueueTest, MultiProducerMultiConsumer) {
  const size_t capacity = 50;
  const size_t items_per_producer = 200;
  const int num_producers = 4;
  const int num_consumers = 4;

  LockedQueue<int> q(capacity);
  std::vector<int> results;
  std::mutex res_mutex;

  std::vector<std::thread> producers;
  producers.reserve(num_producers);
  for (int p = 0; p < num_producers; ++p) {
    producers.emplace_back([&, p]() {
      for (int i = 0; i < items_per_producer; ++i) {
        q.emplace((p * items_per_producer) + i); // blocking emplace
      }
    });
  }

  std::vector<std::thread> consumers;
  consumers.reserve(num_consumers);
  for (int c = 0; c < num_consumers; ++c) {
    consumers.emplace_back([&]() {
      for (int i = 0; i < (num_producers * items_per_producer) / num_consumers;
           i++) {
        std::optional<int> value = q.pop();
        if (!value)
          break;
        std::scoped_lock<std::mutex> lk(res_mutex);
        results.emplace_back(value.value());
      }
    });
  }

  for (auto &t : producers) {
    t.join();
  }
  for (auto &t : consumers) {
    t.join();
  }

  EXPECT_EQ(results.size(), num_producers * items_per_producer);
  std::set<int> unique_items(results.begin(), results.end());
  EXPECT_EQ(unique_items.size(), num_producers * items_per_producer);
}

TEST(Queue, SingleThreadPushPopOrder) {
  LockedQueue<int> q(4);

  q.emplace(10);
  q.emplace(20);
  q.emplace(30);

  EXPECT_EQ(q.pop(), 10);
  EXPECT_EQ(q.pop(), 20);
  EXPECT_EQ(q.pop(), 30);
}

TEST(Queue, WrapAroundCorrectness) {
  LockedQueue<int> q(2);

  q.emplace(1);
  q.emplace(2);

  EXPECT_EQ(q.pop(), 1);

  q.emplace(3); // forces wrap

  EXPECT_EQ(q.pop(), 2);
  EXPECT_EQ(q.pop(), 3);
}
