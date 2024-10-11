#include "./headers/tree_comparer.h"
#include "./headers/tree.h"

int main() {
    Tree firstStandardAST("../asts/first_standard_ast.txt");
    Tree secondStandardAST("../asts/second_standard_ast.txt");

    TreeComparer comparer(firstStandardAST.getRoot(), secondStandardAST.getRoot());
    comparer.printDifferences();
}