#include "./headers/tree_comparer.h"
#include "./headers/tree.h"
#include "./headers/database.h"
#include <iostream>

int main() {
    Tree firstStandardAST("../../asts/first_standard_ast.txt");
    Tree secondStandardAST("../../asts/second_standard_ast.txt");

    TreeComparer comparer(firstStandardAST.getRoot(), secondStandardAST.getRoot());
    comparer.printDifferences();
}