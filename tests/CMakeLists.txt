project(tests)

if (NOT DEFINED gtest_SOURCE_DIR)
    message(FATAL_ERROR "GoogleTest not found")
endif()

FILE(GLOB TEST_SOURCES *.cpp)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} GTest::gtest)
