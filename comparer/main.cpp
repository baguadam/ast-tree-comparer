#include "./headers/tree_comparer.h"
#include "./headers/tree.h"
#include "./headers/database.h"
#include <iostream>

int main() {
    try {
        Tree firstStandardAST("../../asts/vector_98.txt");
        Tree secondStandardAST("../../asts/vector_11.txt");

        TreeComparer comparer(firstStandardAST, secondStandardAST);
        comparer.printDifferences();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}