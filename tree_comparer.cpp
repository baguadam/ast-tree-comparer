#include <iostream>
#include "tree.h"

int main() {
    Tree* firstStandardAST = new Tree("../asts/first_standard_ast.txt");
    Tree* secondStandardAST = new Tree("../asts/second_standard_ast.txt");

    std::cout << firstStandardAST->getRoot()->name << " " << firstStandardAST->getRoot()->value << '\n';
    std::cout << secondStandardAST->getRoot()->name << " " << secondStandardAST->getRoot()->value << '\n';

    delete firstStandardAST;
    delete secondStandardAST;
}   