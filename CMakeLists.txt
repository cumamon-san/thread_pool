cmake_minimum_required(VERSION 3.20)

project(sqlite3_experiments)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fsanitize=thread)
add_link_options(-fsanitize=thread)

add_subdirectory(third-party/googletest)
add_subdirectory(third-party/spdlog)
add_subdirectory(third-party/sqlite3)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${spdlog_SOURCE_DIR}/include)

FILE(GLOB SOURCES src/*.cpp)
FILE(GLOB HEADERS include/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE SQLite3 spdlog::spdlog)

add_subdirectory(tests)

install(TARGETS sqlite3_experiments RUNTIME DESTINATION bin)
