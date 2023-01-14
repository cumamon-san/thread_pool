cmake_minimum_required(VERSION 3.0)

project(sqlite3_experiments)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(sqlite3_experiments main.cpp database.cpp)

target_link_libraries(sqlite3_experiments sqlite3)

install(TARGETS sqlite3_experiments RUNTIME DESTINATION bin)
