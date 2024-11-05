#include "./headers/tree_comparer.h"
#include "./headers/tree.h"
#include "./headers/database.h"
#include "./headers/loggers/tree_comparer_logger.h"
#include "./headers/loggers/console_logger.h"
#include "./headers/factories/console_logger_creator.h"
#include "./headers/factories/database_logger_creator.h"
#include <iostream>

int main() {
    try {
        Tree firstStandardAST("../../asts/vector_98.txt");
        Tree secondStandardAST("../../asts/vector_11.txt");

        // db
        // Database db("../../asts/ast_diff.db3");
        
        // logger
        ConsoleLoggerCreator loggerCreator;
        // DatabaseLoggerCreator loggerCreator(db);
        std::unique_ptr<TreeComparerLogger> logger = loggerCreator.createLogger();

        TreeComparer comparer(firstStandardAST, secondStandardAST, std::move(logger));
        comparer.printDifferences();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}