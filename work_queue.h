#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <functional>
#include <unistd.h>

#include "log.h"
#include "utils.h"
#include "synchronizer.h"

#define WORKER_DEBUG(X) DEBUG("[" << gettid() << "]: " << X)

template<typename T>
class work_queue_t : utils::noncopyable_t {
public:
    using worker_t = std::function<void(T)>;
    work_queue_t(const worker_t& worker, size_t num_workers = 1)
    {
        auto wrapper = [&, worker] (std::stop_token stoken) {
            WORKER_DEBUG("Worker started");
            while (true) {
                std::unique_lock lock(mtx_);
                work_.wait(lock, stoken, [&] { return !queue_.empty() || stoken.stop_requested(); });
                if (stoken.stop_requested()) {
                    WORKER_DEBUG("Stop requested. Exit");
                    return;
                }

                auto item = std::move(queue_.front());
                queue_.pop();
                lock.unlock();

                worker(std::move(item));

                // WORKER_DEBUG("Worker notify done");
                serviced_++;
                work_done_.notify_one();
            }
        };

        for (int i = 0; i < num_workers; ++i) {
            DEBUG("Start worker #" << i);
            workers_.emplace_back(wrapper);
            DEBUG("Worker ID: " << workers_.back().get_id());
        }
    }

    void push(T item) {
        std::lock_guard lock(mtx_);
        DEBUG("Push " << item << " to queue");
        queue_.push(std::move(item));
        pushed_++;
        work_.notify_one();
    }

    size_t pushed() const { return pushed_.load(); };
    size_t serviced() const { return serviced_.load(); };

    void wait() {
        DEBUG("Wait workers");
        std::unique_lock lock(mtx_);
        work_done_.wait(lock, [&] {
            return queue_.empty() && serviced_.load() == pushed_.load();
        });
        lock.unlock();

        for (auto&& worker : workers_) {
            DEBUG("Stop worker with ID " << worker.get_id());
            worker.request_stop();
            worker.join();
        }
        DEBUG("Done");
    }

private:
    std::queue<T> queue_;
    std::vector<std::jthread> workers_;

    std::atomic<size_t> pushed_;
    std::atomic<size_t> serviced_;

    mutable std::mutex mtx_;
    std::condition_variable_any work_;
    std::condition_variable_any work_done_;
};

#undef WORKER_DEBUG

#endif // WORK_QUEUE_H
