#include <iostream>
#include "tree.h"

int main() {
    Tree firstStandardAST("../asts/first_standard_ast.txt");
    Tree secondStandardAST("../asts/second_standard_ast.txt");

    std::cout << firstStandardAST.getRoot()->name << " " << firstStandardAST.getRoot()->value << '\n';
    std::cout << secondStandardAST.getRoot()->name << " " << secondStandardAST.getRoot()->value << '\n';
}   