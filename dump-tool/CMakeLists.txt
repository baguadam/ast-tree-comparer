cmake_minimum_required(VERSION 3.12)
project(dump-tool)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(/usr/lib/llvm-14/include)

add_executable(ast-dump-tool
  main.cpp)

target_link_libraries(ast-dump-tool PRIVATE
  clang-cpp
  LLVM-14)

# Enable generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)