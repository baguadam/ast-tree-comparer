cmake_minimum_required(VERSION 3.14)
project(comparer-tool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include curl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# FetchContent for nlohmann/json to handle JSON
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

include_directories(include)

# libraries for testing
add_library(tree STATIC ./src/tree.cpp)
add_library(tree_comparer STATIC ./src/tree_comparer.cpp)
add_library(utils STATIC ./src/utils.cpp)
add_library(neo4j_wrapper STATIC ./src/neo4j_database_wrapper.cpp)

target_link_libraries(tree_comparer PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(neo4j_wrapper PUBLIC nlohmann_json::nlohmann_json)

target_include_directories(tree PUBLIC include)
target_include_directories(tree_comparer PUBLIC include)
target_include_directories(utils PUBLIC include)
target_include_directories(neo4j_wrapper PUBLIC include)

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# sources
set(SOURCES ./src/main.cpp)

# executables
add_executable(ast-tree-comparer ${SOURCES})
target_link_libraries(ast-tree-comparer PRIVATE tree tree_comparer utils neo4j_wrapper ${CURL_LIBRARIES} nlohmann_json::nlohmann_json)