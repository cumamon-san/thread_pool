cmake_minimum_required(VERSION 3.20)

project(async_pool)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third-party/googletest)
add_subdirectory(third-party/spdlog)

include_directories(${CMAKE_SOURCE_DIR})

FILE(GLOB SOURCES *.cpp)

add_library(core INTERFACE)
target_include_directories(core INTERFACE ${CMAKE_SOURCE_DIR})
target_link_libraries(core INTERFACE spdlog::spdlog)

add_subdirectory(tests)
