#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

#include "spdlog/spdlog.h"
// Should be after
#include "spdlog/fmt/ostr.h"

#include "move_only_function.h"

#define DEBUG_EXPR(X) spdlog::debug(#X " = {}", (X))
#define WORKER_DEBUG(X) spdlog::debug("[{}]: {}", std::this_thread::get_id(), X)

template <typename F, typename... Args>
static auto make_task(F &&f, Args &&...args) {
  return std::packaged_task{
      [t_f = std::forward<F>(f),
       t_args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(std::move(t_f), std::move(t_args));
      }};
}

class thread_pool_t {
public:
  explicit thread_pool_t(
      size_t num_workers = std::thread::hardware_concurrency()) {
    assert(num_workers);
    auto worker_impl = [&](std::stop_token stoken) {
      WORKER_DEBUG("Worker started");
      std::unique_lock lock(mtx_);
      while (true) {
        has_work_.wait(lock, stoken, [&] {
          return !queue_.empty() || stoken.stop_requested();
        });
        if (stoken.stop_requested()) {
          WORKER_DEBUG("Stop requested. Exit");
          return;
        }

        auto task = std::move(queue_.front());
        queue_.pop();
        lock.unlock();

        task();

        lock.lock();
        ++serviced_;
        work_done_.notify_one();
      }
    };

    for (size_t i = 0; i < num_workers; ++i) {
      spdlog::debug("Start worker #{}", i);
      workers_.emplace_back(worker_impl);
      spdlog::debug("Worker ID: {}", workers_.back().get_id());
    }
  }

  thread_pool_t(const thread_pool_t &) = delete;
  thread_pool_t(thread_pool_t &&) = delete;
  thread_pool_t &operator=(const thread_pool_t &) = delete;
  thread_pool_t &operator=(thread_pool_t &&) = delete;

  ~thread_pool_t() { wait(); }

  template <typename F, typename... Args> void push(F &&f, Args &&...args) {
    auto task = make_task(std::forward<F>(f), std::forward<Args>(args)...);
    std::unique_lock lock(mtx_);
    queue_.push([t = std::move(task)]() mutable { t(); });
    ++pushed_;
    lock.unlock();
    has_work_.notify_one();
  }

  template <typename F, typename... Args>
  auto push_with_future(F &&f, Args &&...args) {
    auto task = make_task(std::forward<F>(f), std::forward<Args>(args)...);
    auto future = task.get_future();
    std::unique_lock lock(mtx_);
    queue_.push([t = std::move(task)]() mutable { t(); });
    ++pushed_;
    lock.unlock();
    has_work_.notify_one();
    return future;
  }

  size_t pushed() const {
    std::lock_guard lg(mtx_);
    return pushed_;
  };
  size_t serviced() const {
    std::lock_guard lg(mtx_);
    return serviced_;
  };
  size_t size() const {
    std::lock_guard lg(mtx_);
    return workers_.size();
  };

  void wait() {
    spdlog::debug("Wait workers");
    std::unique_lock lock(mtx_);
    DEBUG_EXPR(pushed_);
    DEBUG_EXPR(queue_.size());
    DEBUG_EXPR(serviced_);
    work_done_.wait(lock,
                    [&] { return queue_.empty() && serviced_ == pushed_; });
    lock.unlock();

    while (!workers_.empty()) {
      auto &w = workers_.back();
      spdlog::debug("Stop worker with ID {}", w.get_id());
      workers_.pop_back();
    }
    spdlog::debug("Done");
  }

private:
  std::queue<utils::move_only_function_t<void()>> queue_;
  std::vector<std::jthread> workers_;

  size_t pushed_ = 0;
  size_t serviced_ = 0;

  mutable std::mutex mtx_;
  std::condition_variable_any has_work_;
  std::condition_variable_any work_done_;
};

#undef DEBUG_EXPR
#undef WORKER_DEBUG

#endif // THREAD_POOL_H
