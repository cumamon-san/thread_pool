#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include <fcntl.h>
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
  fs::path start_directory = fs::canonical("../../..");
  DEBUG_EXPR(start_directory);

  utils::synchronizer_t<std::vector<file_info_t>> results;

  std::unique_ptr<work_queue_t<fs::path>> wq;
  auto handler = [&](fs::path path) {
    if (fs::is_directory(path)) {
      for (auto &&item : fs::directory_iterator(
               path, fs::directory_options::skip_permission_denied))
        if (!fs::is_symlink(item))
          wq->push(item);
    } else if (fs::is_regular_file(path)) {
      utils::scoped_fd_t fd(open(path.c_str(), O_RDONLY));
      if (!fd) {
        ERROR("Cannot open file: " << path << ": " << strerror(errno));
        return;
      }

      file_info_t info(fs::canonical(path), fs::file_size(path));
      results.unique()->push_back(std::move(info));
    }
  };

  wq = std::make_unique<work_queue_t<fs::path>>(handler, 8);

  using clock_t = std::chrono::steady_clock;

  auto start = clock_t::now();
  wq->push(start_directory);
  wq->wait();

  auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                         clock_t::now() - start)
                         .count();
  DEBUG_EXPR(duration_ms);
  DEBUG_EXPR(wq->pushed());
  DEBUG_EXPR(wq->serviced());
  DEBUG("Total " << results.shared()->size() << " files");

  // utils::synchronizer_t<database_t> db(new database_t("index.db"));
  // for (auto&& item : *results.shared())
  //     db.unique()->insert(item.path, item.size);

  // db.shared()->print();

  return EXIT_SUCCESS;
}
