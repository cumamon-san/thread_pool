project(tests)

FILE(GLOB TEST_SOURCES *.cpp)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE core GTest::gtest)

add_executable(util_tests main_tests.cpp util_tests.cpp)
target_link_libraries(util_tests PRIVATE core GTest::gtest)
