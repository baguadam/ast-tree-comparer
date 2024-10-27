cmake_minimum_required(VERSION 3.12)
project(comparer-tool)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(/usr/lib/llvm-14/include)

# SQLiteCpp as a subdirectory, so its CMakeLists.txt gets invoked
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/SQLiteCpp)

# specifying the headers
include_directories(headers)

# specifying the sources
set(SOURCES tree.cpp tree_comparer.cpp database.cpp utils.cpp main.cpp)

add_executable(ast-tree-comparer ${SOURCES})

# link agains the necessary libraries
target_link_libraries(ast-tree-comparer
  clang-cpp
  LLVM-14
  SQLiteCpp
  sqlite3
  pthread
  dl)
