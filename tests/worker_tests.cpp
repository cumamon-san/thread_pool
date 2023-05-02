#include <gtest/gtest.h>

#include <chrono>

#include "worker_pool.h"

TEST(WorkerPool, WaitOne) {
    worker_pool_t pool(1);
    pool.push([] {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    });
    pool.wait();
}

TEST(WorkerPool, WaitMany) {
    worker_pool_t pool;
    for(size_t i = 0; i < pool.size() * 2; ++i)
        pool.push([] {
            std::this_thread::sleep_for(std::chrono::seconds(3));
        });
    pool.wait();
}

TEST(WorkerPool, Simple) {
    worker_pool_t pool(1);
    std::atomic<int> counter;
    for(int i = 0; i < 1'000; ++i)
        pool.push([&counter]{ counter++; });
    pool.wait();
    ASSERT_EQ(pool.pushed(), 1'000);
    ASSERT_EQ(pool.serviced(), 1'000);
    ASSERT_EQ(counter.load(), 1'000);
}

TEST(WorkerPool, MultiThread) {
    std::atomic<int> counter;
    worker_pool_t pool(4);
    for(int i = 0; i < 1'000; ++i)
        pool.push([&counter]{ counter++; });
    pool.wait();
    ASSERT_EQ(pool.pushed(), 1'000);
    ASSERT_EQ(pool.serviced(), 1'000);
    ASSERT_EQ(counter.load(), 1'000);
}

TEST(WorkerPool, MultiThread2) {
    std::atomic<int> counter;
    worker_pool_t pool(4);
    worker_pool_t producer_pool(4);
    for(int i = 0; i < producer_pool.size(); ++i) {
        producer_pool.push([&counter, &pool] {
            for(int j = 0; j < 1'000; ++j) {
                pool.push([&counter]{ counter++; });
                pool.push([&counter]{ counter--; });
            }
        });
    }
    producer_pool.wait();
    pool.wait();
    ASSERT_EQ(pool.serviced(), 8'000);
    ASSERT_EQ(counter.load(), 0);
}

TEST(WorkerPool, Futures) {
    worker_pool_t pool(1);
    auto f_int = pool.push_with_future([]{ return 1; });
    auto f_str = pool.push_with_future([]{ return std::string("str"); });
    pool.wait();
    ASSERT_EQ(pool.serviced(), 2);
    ASSERT_EQ(f_int.get(), 1);
    ASSERT_EQ(f_str.get(), "str");
}

TEST(WorkerPool, Exceptions) {
    worker_pool_t pool(1);
    pool.push([]{ throw 0; });
    auto f1 = pool.push_with_future([]{ throw 1; });
    auto f2 = pool.push_with_future([]{ return 2; });
    pool.wait();
    ASSERT_EQ(pool.serviced(), 3);
    ASSERT_EQ(f2.get(), 2);
    try {
        f1.get();
    }
    catch (int i) {
        ASSERT_EQ(i, 1);
    }
}

static int sum(int a, int b) {
    return a + b;
}

TEST(WorkerPool, ForwardToPush) {
    worker_pool_t pool;
    pool.push(sum, 0, 0);
    auto f1 = pool.push_with_future(sum, 0, 1);
    auto f2 = pool.push_with_future(&sum, 1, 1);
    auto f3 = pool.push_with_future([](auto ptr) { return *ptr; }, std::make_unique<int>(3));
    auto f4 = pool.push_with_future(std::bind(sum, 1, 3));
    auto f5 = pool.push_with_future([ptr = std::make_unique<int>(5)] { return *ptr; });
    ASSERT_EQ(f1.get(), 1);
    ASSERT_EQ(f2.get(), 2);
    ASSERT_EQ(f3.get(), 3);
    ASSERT_EQ(f4.get(), 4);
    ASSERT_EQ(f5.get(), 5);
}
