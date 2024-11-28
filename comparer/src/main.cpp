#include "../include/tree_comparer.h"
#include "../include/tree.h"
#include <iostream>

bool testDatabaseConnection(Neo4jDatabaseWrapper& dbWrapper) {
    try {
        dbWrapper.clearDatabase();
        dbWrapper.createIndices();
        return true; // connection successful
    } catch (const std::exception& e) {
        std::cerr << "Database connection test failed: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <first_ast_file> <second_ast_file>" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string firstFilePath = argv[1];
    const std::string secondFilePath = argv[2];
    const char* neo4jPassword = std::getenv("NEO4J_PASSWORD");
    if (!neo4jPassword) {
        std::cerr << "NEO4J_PASSWORD environment variable not set, using default value" << std::endl;
        neo4jPassword = "default_password";
    }

    try {
        Tree firstStandardAST(firstFilePath);
        Tree secondStandardAST(secondFilePath);

        // db wrapper
        Neo4jDatabaseWrapper dbWrapper("http://localhost:7474", "neo4j", neo4jPassword);
        if (!testDatabaseConnection(dbWrapper)) {
            std::cerr << "Failed to connect to Neo4j database. Terminating program." << std::endl; 
            return EXIT_FAILURE;
        }

        TreeComparer comparer(firstStandardAST, secondStandardAST, dbWrapper);
        comparer.printDifferences();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}