#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <unistd.h>
#include <vector>

#include "log.h"
#include "synchronizer.h"
#include "utils.h"

#define WORKER_DEBUG(X) DEBUG("[" << std::this_thread::get_id() << "]: " << X)

template <typename T> class work_queue_t : utils::noncopyable_t {
public:
  using handler_t = std::function<void(T)>;
  explicit work_queue_t(const handler_t &handler, size_t num_workers = 1) {
    auto wrapper = [&, handler](std::stop_token stoken) {
      WORKER_DEBUG("Worker started");
      while (true) {
        std::unique_lock lock(mtx_);
        has_work_.wait(lock, stoken,
                   [&] { return !queue_.empty() || stoken.stop_requested(); });
        if (stoken.stop_requested()) {
          WORKER_DEBUG("Stop requested. Exit");
          return;
        }

        auto item = std::move(queue_.front());
        queue_.pop();
        lock.unlock();

        handler(std::move(item));

        serviced_++;
        work_done_.notify_one();
      }
    };

    for (int i = 0; i < num_workers; ++i) {
      DEBUG("Start worker #" << i);
      workers_.emplace(wrapper);
      DEBUG("Worker ID: " << workers_.back().get_id());
    }
  }

  ~work_queue_t() { wait(); }

  void push(T item) {
    std::unique_lock lock(mtx_);
    queue_.push(std::move(item));
    ++pushed_;
    if (pushed_ % 10000 == 0)
      DEBUG("Push " << pushed_ << " items");
    has_work_.notify_one();
  }

  template <typename Container> void push_many(Container items) {
    std::unique_lock lock(mtx_);
    for (auto &&item : items) {
      queue_.push(std::move(item));
      pushed_++;
      if (pushed_ % 10000 == 0)
        DEBUG("Push " << pushed_ << " items");
    }
    has_work_.notify_all();
  }

  size_t pushed() const { return pushed_; };
  size_t serviced() const { return serviced_.load(); };

  void wait() {
    DEBUG("Wait workers");
    std::unique_lock lock(mtx_);
    work_done_.wait(
        lock, [&] { return queue_.empty() && serviced_.load() == pushed_; });
    lock.unlock();

    while (!workers_.empty()) {
      auto &w = workers_.front();
      DEBUG("Stop worker with ID " << w.get_id());
      w.request_stop();
      w.join();
      workers_.pop();
    }
    DEBUG("Done");
  }

private:
  std::queue<T> queue_;
  std::queue<std::jthread> workers_;

  size_t pushed_ = 0;
  std::atomic<size_t> serviced_;

  mutable std::mutex mtx_;
  std::condition_variable_any has_work_;
  std::condition_variable_any work_done_;
};

#undef WORKER_DEBUG

#endif // WORK_QUEUE_H
