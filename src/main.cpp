#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>

#include "spdlog/spdlog.h"

#define DEBUG_EXPR(X) spdlog::debug(#X " = {}", (X))

int main() {
  spdlog::set_pattern("[%^%l%$] %v");
  spdlog::set_level(spdlog::level::debug);

  DEBUG_EXPR("From main");

  return EXIT_SUCCESS;
}
