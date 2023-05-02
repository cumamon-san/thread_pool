cmake_minimum_required(VERSION 3.0)

project(sqlite3_experiments)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

add_compile_options(-fsanitize=thread)
add_link_options(-fsanitize=thread)

FILE(GLOB SOURCES *.cpp)
FILE(GLOB HEADERS *.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(sqlite3_experiments sqlite3)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(tests)

install(TARGETS sqlite3_experiments RUNTIME DESTINATION bin)
