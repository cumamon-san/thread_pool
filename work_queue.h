#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <functional>

#include "log.h"
#include "utils.h"
#include "synchronizer.h"

template<typename T>
class work_queue_t : utils::noncopyable_t {
public:
    using worker_t = std::function<void(T)>;
    work_queue_t(const worker_t& worker, size_t num_workers = 1)
    {
        auto wrapper = [&, worker] (std::stop_token stoken) {
            while (true) {
                std::unique_lock lock(mtx_);
                work_.wait(lock, stoken, [&] { return queue_.size() > 0 || stoken.stop_requested(); });
                if (stoken.stop_requested()) {
                    DEBUG("Stop requested. Exit");
                    return;
                }

                auto item = std::move(queue_.front());
                queue_.pop();
                lock.unlock();

                worker(std::move(item));

                work_done_.notify_one();
                DEBUG("Worker notify done");
            }
        };

        for (int i = 0; i < num_workers; ++i) {
            DEBUG("Start worker #" << i);
            workers_.emplace_back(wrapper);
        }
    }

    void push(T item) {
        std::lock_guard lock(mtx_);
        DEBUG("Push " << item << " to queue");
        queue_.push(std::move(item));
        work_.notify_one();
    }

    void wait() {
        DEBUG("Wait workers");
        std::unique_lock lock(mtx_);
        work_done_.wait(lock, [&] { return queue_.size() == 0; });
        lock.unlock();

        for (int i = 0; i < workers_.size(); ++i) {
            DEBUG("Stop worker #" << i);
            workers_.at(i).request_stop();
            workers_.at(i).join();
        }
        DEBUG("Done");
    }

private:
    std::queue<T> queue_;
    std::vector<std::jthread> workers_;

    mutable std::mutex mtx_;
    std::condition_variable_any work_;
    std::condition_variable_any work_done_;
};

#endif // WORK_QUEUE_H
