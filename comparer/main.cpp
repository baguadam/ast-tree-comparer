#include "./headers/tree_comparer.h"
#include "./headers/tree.h"
#include "./headers/loggers/tree_comparer_logger.h"
#include "./headers/loggers/console_logger.h"
#include "./headers/factories/console_logger_creator.h"
#include <iostream>

int main() {
    try {
        Tree firstStandardAST("../../asts/first_standard_ast.txt");
        Tree secondStandardAST("../../asts/second_standard_ast.txt");
        
        // logger
        // ConsoleLoggerCreator loggerCreator;
        // std::unique_ptr<TreeComparerLogger> logger = loggerCreator.createLogger();

        TreeComparer comparer(firstStandardAST, secondStandardAST);
        comparer.printDifferences();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}