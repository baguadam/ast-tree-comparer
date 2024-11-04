#include "./headers/tree_comparer.h"
#include "./headers/tree.h"
#include "./headers/database.h"
#include "./headers/tree_comparer_logger.h"
#include "./headers/console_logger.h"
#include <iostream>

int main() {
    try {
        Tree firstStandardAST("../../asts/first_standard_ast.txt");
        Tree secondStandardAST("../../asts/second_standard_ast.txt");
        TreeComparerLogger logger = ConsoleLogger();

        TreeComparer comparer(firstStandardAST, secondStandardAST, logger);
        comparer.printDifferences();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}