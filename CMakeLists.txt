cmake_minimum_required(VERSION 3.0)

project(sqlite3_experiments)

add_executable(sqlite3_experiments main.cpp database.cpp)

target_link_libraries(sqlite3_experiments sqlite3)

install(TARGETS sqlite3_experiments RUNTIME DESTINATION bin)
