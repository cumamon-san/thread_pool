#include <filesystem>
#include <future>
#include <string>
#include <vector>
#include <optional>

#include "database.h"
#include "log.h"
#include "synchronizer.h"
#include "work_queue.h"

namespace fs = std::filesystem;

int main() {
    utils::synchronizer_t<std::vector<fs::path>> paths;
    for(auto&& item : fs::recursive_directory_iterator("..")) {
        if(fs::is_regular_file(item))
            paths.unique()->push_back(item);
    }

    utils::synchronizer_t<database_t> db(new database_t("index.db"));
    utils::synchronizer_t<std::vector<std::pair<fs::path, size_t>>> results;

    auto get_path = [&] {
        auto uniq_paths = paths.unique();
        if(!uniq_paths->empty()) {
            auto path = std::move(uniq_paths->back());
            uniq_paths->pop_back();
            return std::optional<fs::path> {path};
        }
        return std::optional<fs::path> {};
    };

    std::vector<std::future<void>> futures;
    for(int i = 0; i < 3; ++i) {
        futures.push_back(std::async(std::launch::async, [&] {
            std::vector<std::pair<fs::path, size_t>> local_result;
            while(auto path = get_path())
                local_result.emplace_back(*path, fs::file_size(*path));
            DEBUG("Handle " << local_result.size() << " files");
            auto uniq_result = results.unique();
            uniq_result->insert(uniq_result->end(), local_result.begin(), local_result.end());
        }));
    }

    for(auto&& f : futures)
        f.wait();

    DEBUG("Total " << results.shared()->size() << " files");

    for(auto&& item : *results.shared())
        db.unique()->insert(item.first, item.second);

    db.unique()->print();

    work_queue_t wq(3);
    for (int i = 0; i < 10; ++i)
        wq.push(10);
    wq.wait();
    DEBUG_EXPR(wq.sum());

    return 0;
}
