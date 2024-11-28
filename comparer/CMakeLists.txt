cmake_minimum_required(VERSION 3.12)
project(comparer-tool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})
include_directories(include)

# create a library from tree.cpp and tree_comparer.cpp for testing 
add_library(tree STATIC ./src/tree.cpp)
add_library(tree_comparer STATIC ./src/tree_comparer.cpp)
add_library(utils STATIC ./src/utils.cpp)

target_include_directories(tree PUBLIC include)
target_include_directories(tree_comparer PUBLIC include)
target_include_directories(utils PUBLIC include)

set(SOURCES ./src/neo4j_database_wrapper.cpp ./src/main.cpp)

add_executable(ast-tree-comparer ${SOURCES})
target_link_libraries(ast-tree-comparer PRIVATE tree tree_comparer utils ${CURL_LIBRARIES})