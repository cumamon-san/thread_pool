cmake_minimum_required(VERSION 3.20)

project(sqlite3_experiments)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third-party/googletest)
add_subdirectory(third-party/spdlog)
add_subdirectory(third-party/sqlite3)

include_directories(${CMAKE_SOURCE_DIR})

FILE(GLOB SOURCES *.cpp)

add_library(core INTERFACE)
target_include_directories(core INTERFACE ${CMAKE_SOURCE_DIR})
target_link_libraries(core INTERFACE SQLite3 spdlog::spdlog)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE core)

add_subdirectory(tests)

install(TARGETS sqlite3_experiments RUNTIME DESTINATION bin)
