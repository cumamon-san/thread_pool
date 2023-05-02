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
#include "worker_pool.h"

namespace fs = std::filesystem;

int sum(int a, int b) {
  return a + b;
}

int main() {
  return EXIT_SUCCESS;
}
