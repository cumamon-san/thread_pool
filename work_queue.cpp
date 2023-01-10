#include "work_queue.h"

#include "log.h"

work_queue_t::work_queue_t(size_t num_workers)
: sum_(new size_t(0))
{
    auto worker = [&] (std::stop_token stoken) {
        while (true) {
            std::unique_lock lock(mtx_);
            work_.wait(lock, stoken, [&] { return queue_.size() > 0 || stoken.stop_requested(); });
            if (stoken.stop_requested()) {
                DEBUG("Stop requested. Exit");
                return;
            }

            // there is work to be done - pretend we're working on something
            int x = queue_.front();
            queue_.pop();
            lock.unlock();

            *sum_.unique() += x;

            work_done_.notify_one();
            DEBUG("Worker notify done");
        }
    };

    for (int i = 0; i < num_workers; ++i) {
        DEBUG("Start worker #" << i);
        workers_.emplace_back(worker);
    }
}

void work_queue_t::push(int val) {
    std::lock_guard lock(mtx_);
    DEBUG("Push " << val << " to queue");
    queue_.push(val);
    work_.notify_one();
}

void work_queue_t::wait() {
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
