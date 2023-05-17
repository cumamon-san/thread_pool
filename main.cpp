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

#include "move_only_function.h"
#include "utils.h"

#define DEBUG_EXPR(X) spdlog::debug(#X " = {}", (X))

int print() {
  spdlog::warn("From print");
  return 0;
}

int main() {
  spdlog::set_pattern("[%^%l%$] %v");
  spdlog::set_level(spdlog::level::debug);

  // DEBUG_EXPR("From main");

  {
    utils::move_only_function_t<int()> h(&print);
    h();
    h = [] {
      spdlog::warn("From int()");
      return 56;
    };
    h();
  }

  {
    utils::move_only_function_t<void()> h([] { spdlog::warn("From lambda1"); }),
        h2([] { spdlog::warn("From lambda2"); });
    h();
    h = std::move(h2);
    h();
  }

  return EXIT_SUCCESS;
}
