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
#include "file_info.h"
#include "log.h"
#include "synchronizer.h"
#include "work_queue.h"

namespace fs = std::filesystem;

int main() {
  fs::path start_directory = fs::canonical("../../..");
  DEBUG_EXPR(start_directory);

  utils::synchronizer_t<std::vector<file_info_t>> results;

  std::unique_ptr<work_queue_t<fs::path>> wq;
  auto handler = [&](fs::path path) {
    utils::scoped_fd_t fd(open(path.c_str(), O_RDONLY));
    if (!fd) {
      ERROR("Cannot open file: " << path << ": " << strerror(errno));
      return;
    }

    std::array<char, 512> buff;
    if(read(fd, buff.data(), buff.size()) < 0) {
      ERROR("Cannot read file: " << path << ": " << strerror(errno));
      return;
    }

    file_info_t info(fs::canonical(path), fs::file_size(path));
    results.unique()->push_back(std::move(info));
  };

  wq = std::make_unique<work_queue_t<fs::path>>(handler, 8);

  using clock_t = std::chrono::steady_clock;
  auto start = clock_t::now();

  for (const auto& item : fs::recursive_directory_iterator(start_directory, fs::directory_options::skip_permission_denied))
    if (fs::is_regular_file(item))
      wq->push(item);
  wq->wait();

  auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                         clock_t::now() - start)
                         .count();
  DEBUG_EXPR(duration_ms);
  DEBUG_EXPR(wq->pushed());
  DEBUG_EXPR(wq->serviced());
  DEBUG("Total " << results.shared()->size() << " files");

  database_t db("index.db");

  start = clock_t::now();
  db.insert_many(*results.shared(), 10000);
  duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock_t::now() - start).count();
  DEBUG_EXPR(duration_ms);


  return EXIT_SUCCESS;
}
