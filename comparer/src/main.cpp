#include "../include/tree_comparer.h"
#include "../include/tree.h"
// #include "./headers/loggers/tree_comparer_logger.h"
// #include "./headers/loggers/console_logger.h"
// #include "./headers/factories/console_logger_creator.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <first_ast_file> <second_ast_file>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string firstFilePath = argv[1];
    std::string secondFilePath = argv[2];

    try {
        Tree firstStandardAST(firstFilePath);
        Tree secondStandardAST(secondFilePath);

        // db wrapper
        Neo4jDatabaseWrapper dbWrapper("http://localhost:7474", "neo4j", "eszter2005");

        TreeComparer comparer(firstStandardAST, secondStandardAST, dbWrapper);
        comparer.printDifferences();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}