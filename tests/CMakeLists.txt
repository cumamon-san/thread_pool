project(tests)

find_package(GTest)

enable_testing()

FILE(GLOB TEST_SOURCES *.cpp)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(tests GTest::gtest_main)
