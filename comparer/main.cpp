#include "./headers/tree_comparer.h"
#include "./headers/tree.h"
#include "./headers/database.h"
#include <iostream>
#include <filesystem>

void testDatabaseOperations() {
    // Ensure the directory exists
    std::string dbPath = "../db/testdb.sqlite3";
    std::filesystem::path dbDir = std::filesystem::path(dbPath).parent_path();
    if (!std::filesystem::exists(dbDir)) {
        std::cout << "Creating directory: " << dbDir << std::endl;
        std::filesystem::create_directories(dbDir);
    }

    // Convert to absolute path
    std::filesystem::path absDbPath = std::filesystem::absolute(dbPath);
    std::cout << "Absolute database path: " << absDbPath << std::endl;

    // // Check if the database file already exists
    // if (std::filesystem::exists(absDbPath)) {
    //     std::cout << "Database file already exists, deleting it: " << absDbPath << std::endl;
    //     std::filesystem::remove(absDbPath);
    // }

    // Create a database object
    std::cout << "Creating database object..." << std::endl;
    try {
        Database db(absDbPath.string());
    } catch (const std::exception& e) {
        std::cerr << "Error creating database object: " << e.what() << std::endl;
        return;
    }

    std::cout << "Database operations completed successfully.\n";
}

int main() {
    testDatabaseOperations();

    // Tree firstStandardAST("../../asts/first_standard_ast.txt");
    // Tree secondStandardAST("../../asts/second_standard_ast.txt");

    // TreeComparer comparer(firstStandardAST.getRoot(), secondStandardAST.getRoot());
    // comparer.printDifferences();
}