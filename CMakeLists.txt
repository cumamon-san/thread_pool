cmake_minimum_required(VERSION 3.0)

project(sqlite3_experiments)

include_directories("sqlite3")

add_executable(sqlite3_experiments main.cpp sqlite3/sqlite3.c)

install(TARGETS sqlite3_experiments RUNTIME DESTINATION bin)
