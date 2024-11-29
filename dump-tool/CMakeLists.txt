cmake_minimum_required(VERSION 3.12)
project(ClangASTTool)

# find LLVM and Clang libraries and components
find_package(Clang REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add executable
add_executable(clang_ast_tool main.cpp)

# link LLVM and Clang libraries
target_link_libraries(clang_ast_tool
  PRIVATE
  clang-cpp
  LLVM
)