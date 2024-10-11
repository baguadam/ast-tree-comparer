#include <iostream>
#include "node_reader.h"

int main() {
    NodeReader reader;

    Node* firstStandardAST = reader.readASTDump("../asts/first_standard_ast.txt");
    Node* secondStandardAST = reader.readASTDump("../asts/second_standard_ast.txt");

    if (!firstStandardAST || !secondStandardAST) {
        std::cerr << "Error while opening files! \n";
        return -1;
    }

    std::cout << firstStandardAST->name << " " << firstStandardAST->value << '\n';
    std::cout << secondStandardAST->name << " " << secondStandardAST->value << '\n';
}   