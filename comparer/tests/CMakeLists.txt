cmake_minimum_required(VERSION 3.14)
project(ast_tree_comparer_tests VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent for Googletest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# FetchContent for nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

set(TEST_SOURCES
    tree_comparer_test.cpp
    tree_test.cpp
    integration_test.cpp
)

add_executable(test_all ${TEST_SOURCES})

target_link_libraries(test_all PRIVATE gtest gmock gtest_main)

target_include_directories(test_all PRIVATE ../include)

target_link_libraries(test_all
  PRIVATE
  ${CMAKE_SOURCE_DIR}/../build/libtree.a 
  ${CMAKE_SOURCE_DIR}/../build/libtree_comparer.a
  ${CMAKE_SOURCE_DIR}/../build/libutils.a
  nlohmann_json::nlohmann_json
)

enable_testing()
add_test(NAME test_all COMMAND test_all)