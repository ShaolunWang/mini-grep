#include "queue/queue.h"
#include "gtest/gtest.h"
#include <mutex>
#include <thread>

TEST(QueueTest, SingleThreadPushPop) {
  Queue<int> q(4);

  q.push(10);
  q.push(20);

  int value;
  value = q.pop();
  EXPECT_EQ(value, 10);

  value = q.pop();
  EXPECT_EQ(value, 20);
}

TEST(QueueTest, MultiThreadPushPop) {
  const size_t capacity = 10;
  const size_t num_items = 1000;

  Queue<int> q(capacity);

  std::vector<int> results;
  std::mutex res_mutex;

  std::thread producer([&]() {
    for (int i = 0; i < num_items; ++i) {
      q.push(i);
    }
  });

  // Consumer thread
  std::thread consumer([&]() {
    for (int i = 0; i < num_items; ++i) {
      int v = q.pop(); // blocking pop
      std::scoped_lock<std::mutex> lk(res_mutex);
      results.push_back(v);
    }
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(results.size(), num_items);
  for (int i = 0; i < num_items; ++i) {
    EXPECT_EQ(results[i], i);
  }
}

TEST(QueueTest, MultiProducerMultiConsumer) {
  const size_t capacity = 50;
  const size_t items_per_producer = 200;
  const int num_producers = 4;
  const int num_consumers = 4;

  Queue<int> q(capacity);
  std::vector<int> results;
  std::mutex res_mutex;

  // spawn a bunch of producers
  std::vector<std::thread> producers;
  for (int p = 0; p < num_producers; ++p) {
    producers.emplace_back([&, p]() {
      for (int i = 0; i < items_per_producer; ++i) {
        q.push(p * items_per_producer + i); // blocking push
      }
    });
  }

  // spawn a bunch of consumers
  std::vector<std::thread> consumers;
  for (int c = 0; c < num_consumers; ++c) {
    consumers.emplace_back([&]() {
      for (int i = 0; i < (num_producers * items_per_producer) / num_consumers;
           i++) {
        int value = q.pop(); // blocking pop
        std::scoped_lock<std::mutex> lk(res_mutex);
        results.push_back(value);
      }
    });
  }

  // cleanup join
  for (auto &t : producers) {
    t.join();
  }
  for (auto &t : consumers) {
    t.join();
  }

  // Verify the total number of items / unique items
  EXPECT_EQ(results.size(), num_producers * items_per_producer);
  std::set<int> unique_items(results.begin(), results.end());
  EXPECT_EQ(unique_items.size(), num_producers * items_per_producer);
}

TEST(Queue, SingleThread_PushPop_Order) {
  Queue<int> q(4);

  q.push(10);
  q.push(20);
  q.push(30);

  EXPECT_EQ(q.pop(), 10);
  EXPECT_EQ(q.pop(), 20);
  EXPECT_EQ(q.pop(), 30);
}

TEST(Queue, WrapAround_Correctness) {
  Queue<int> q(2);

  q.push(1);
  q.push(2);

  EXPECT_EQ(q.pop(), 1);

  q.push(3); // forces wrap

  EXPECT_EQ(q.pop(), 2);
  EXPECT_EQ(q.pop(), 3);
}
