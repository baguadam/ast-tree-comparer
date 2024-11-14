cmake_minimum_required(VERSION 3.12)
project(comparer-tool)

find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})
include_directories(headers)

# create a library from tree.cpp and tree_comparer.cpp for testing 
add_library(tree STATIC tree.cpp)
add_library(tree_comparer STATIC tree_comparer.cpp)
add_library(utils STATIC utils.cpp)

target_include_directories(tree PUBLIC headers)
target_include_directories(tree_comparer PUBLIC headers)
target_include_directories(utils PUBLIC headers)

set(SOURCES neo4j_database_wrapper.cpp ./loggers/console_logger.cpp main.cpp)

add_executable(ast-tree-comparer ${SOURCES})
target_link_libraries(ast-tree-comparer PRIVATE tree tree_comparer utils ${CURL_LIBRARIES})