cmake_minimum_required(VERSION 3.12)
project(comparer-tool)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(CURL REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})

# specifying the headers
include_directories(headers)

# specifying the sources
set(SOURCES tree.cpp tree_comparer.cpp utils.cpp neo4j_database_wrapper.cpp ./loggers/console_logger.cpp main.cpp)

add_executable(ast-tree-comparer ${SOURCES})

# link agains the necessary libraries
target_link_libraries(ast-tree-comparer PRIVATE ${CURL_LIBRARIES})