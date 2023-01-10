#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <condition_variable>
#include <deque>
#include <queue>
#include <mutex>
#include <thread>
#include <vector>

#include "utils.h"
#include "synchronizer.h"

class work_queue_t : utils::noncopyable_t {
public:
    work_queue_t();
    void push(int val);
    size_t sum() const { std::unique_lock lock(sum_mtx_); return sum_; }
    void wait();

private:
    std::queue<int> queue_;
    std::vector<std::jthread> workers_;

    mutable std::mutex mtx_;
    std::condition_variable_any work_;
    std::condition_variable_any work_done_;
    size_t sum_;
    mutable std::mutex sum_mtx_;
};

#endif // WORK_QUEUE_H
