cmake_minimum_required(VERSION 3.14)
project(ast_tree_comparer_tests VERSION 1.0 LANGUAGES CXX)

# Include FetchContent to download GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)


FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
    tree_comparer_test.cpp
    tree_test.cpp
)

# Create a single executable for all tests
add_executable(test_all ${TEST_SOURCES})

# Link against Google Test and other project libraries
target_link_libraries(test_all gtest gtest_main project_lib)
target_include_directories(test_all PRIVATE ../headers)

enable_testing()
add_test(NAME test_all COMMAND test_all)
