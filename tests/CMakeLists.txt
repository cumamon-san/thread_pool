project(tests)

FILE(GLOB TEST_SOURCES *.cpp)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE core GTest::gtest)
