#include <chrono>
#include <filesystem>
#include <future>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include <sys/stat.h>

#include "database.h"
#include "log.h"
#include "synchronizer.h"
#include "work_queue.h"

namespace fs = std::filesystem;

struct file_info_t {
    file_info_t(fs::path p, size_t s) : path(std::move(p)), size(s) {}
    fs::path path;
    size_t size;
};

int main() {
    utils::synchronizer_t<std::vector<file_info_t>> results;

    auto worker = [&] (fs::path path) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        struct stat st;
        stat(path.c_str(), &st);
        results.unique()->emplace_back(fs::canonical(path), st.st_size);
    };

    using clock_t = std::chrono::steady_clock;
    auto start = clock_t::now();
    work_queue_t<fs::path> wq(worker, 2);
    for (auto&& item : fs::recursive_directory_iterator("../../..", fs::directory_options::skip_permission_denied)) {
        if (fs::is_regular_file(item))
            wq.push(item);
    }
    wq.wait();

    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock_t::now() - start).count();
    DEBUG_EXPR(duration_ms);
    DEBUG_EXPR(wq.pushed());
    DEBUG_EXPR(wq.serviced());
    DEBUG("Total " << results.shared()->size() << " files");

    // utils::synchronizer_t<database_t> db(new database_t("index.db"));
    // for (auto&& item : *results.shared())
    //     db.unique()->insert(item.path, item.size);

    // db.shared()->print();

    return 0;
}
