#include "gtest/gtest.h"

#include <string_view>

#include "spdlog/spdlog.h"

using namespace std::string_view_literals;

static void init_logger(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == "--verbose"sv) {
            spdlog::set_pattern("[From testd function]: %v");
            spdlog::set_level(spdlog::level::trace);
            return;
        }
    }
    spdlog::set_level(spdlog::level::off);
}

int main(int argc, char **argv) {
    init_logger(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
