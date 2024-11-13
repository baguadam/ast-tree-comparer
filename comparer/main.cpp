#include "./headers/tree_comparer.h"
#include "./headers/tree.h"
#include "./headers/loggers/tree_comparer_logger.h"
#include "./headers/loggers/console_logger.h"
#include "./headers/factories/console_logger_creator.h"
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
        
        // logger
        ConsoleLoggerCreator loggerCreator;
        std::unique_ptr<TreeComparerLogger> logger = loggerCreator.createLogger();

        TreeComparer comparer(firstStandardAST, secondStandardAST, std::move(logger));
        comparer.printDifferences();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}