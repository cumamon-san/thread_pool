#include "gtest/gtest.h"

#include <chrono>

#include "thread_pool.h"

TEST(ThreadPool, WaitEmpty) { thread_pool_t().wait(); }

TEST(ThreadPool, WaitOne) {
  thread_pool_t pool(1);
  pool.push([] { std::this_thread::sleep_for(std::chrono::seconds(1)); });
  pool.wait();
}

TEST(ThreadPool, WaitMany) {
  thread_pool_t pool;
  for (size_t i = 0; i < pool.size() * 2; ++i)
    pool.push([] { std::this_thread::sleep_for(std::chrono::seconds(2)); });
  pool.wait();
}

TEST(ThreadPool, Simple) {
  thread_pool_t pool(1);
  std::atomic<int> counter;
  for (int i = 0; i < 1'000; ++i)
    pool.push([&counter] { counter++; });
  pool.wait();
  EXPECT_EQ(pool.pushed(), 1'000);
  EXPECT_EQ(pool.serviced(), 1'000);
  EXPECT_EQ(counter.load(), 1'000);
}

TEST(ThreadPool, MultiThread) {
  std::atomic<int> counter;
  thread_pool_t pool(4);
  for (int i = 0; i < 1'000; ++i)
    pool.push([&counter] { counter++; });
  pool.wait();
  EXPECT_EQ(pool.pushed(), 1'000);
  EXPECT_EQ(pool.serviced(), 1'000);
  EXPECT_EQ(counter.load(), 1'000);
}

TEST(ThreadPool, MultiThread2) {
  std::atomic<int> counter;
  thread_pool_t pool(4);
  thread_pool_t producer_pool(4);
  for (size_t i = 0; i < producer_pool.size(); ++i) {
    producer_pool.push([&counter, &pool] {
      for (int j = 0; j < 1'000; ++j) {
        pool.push([&counter] { counter++; });
        pool.push([&counter] { counter--; });
      }
    });
  }
  producer_pool.wait();
  pool.wait();
  EXPECT_EQ(pool.serviced(), 8'000);
  EXPECT_EQ(counter.load(), 0);
}

TEST(ThreadPool, Futures) {
  thread_pool_t pool(1);
  auto f_int = pool.push_with_future([] { return 1; });
  auto f_str = pool.push_with_future([] { return std::string("str"); });
  pool.wait();
  EXPECT_EQ(pool.serviced(), 2);
  EXPECT_EQ(f_int.get(), 1);
  EXPECT_EQ(f_str.get(), "str");
}

TEST(ThreadPool, Exceptions) {
  thread_pool_t pool(1);
  pool.push([] { throw 0; });
  auto f1 = pool.push_with_future([] { throw 1; });
  auto f2 = pool.push_with_future([] { return 2; });
  pool.wait();
  EXPECT_EQ(pool.serviced(), 3);
  EXPECT_EQ(f2.get(), 2);
  try {
    f1.get();
  } catch (int i) {
    EXPECT_EQ(i, 1);
  }
}

static int sum(int a, int b) { return a + b; }

TEST(ThreadPool, ForwardToPush) {
  thread_pool_t pool;
  pool.push(sum, 0, 0);
  auto f1 = pool.push_with_future(sum, 0, 1);
  auto f2 = pool.push_with_future(&sum, 1, 1);
  auto f3 = pool.push_with_future([](auto ptr) { return *ptr; },
                                  std::make_unique<int>(3));
  auto f4 = pool.push_with_future(std::bind(sum, 1, 3));
  auto f5 =
      pool.push_with_future([ptr = std::make_unique<int>(5)] { return *ptr; });
  EXPECT_EQ(f1.get(), 1);
  EXPECT_EQ(f2.get(), 2);
  EXPECT_EQ(f3.get(), 3);
  EXPECT_EQ(f4.get(), 4);
  EXPECT_EQ(f5.get(), 5);
}
