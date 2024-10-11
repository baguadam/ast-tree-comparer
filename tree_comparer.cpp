#include <iostream>
#include "node_reader.h"

int main() {
    NodeReader reader;

    std::unique_ptr<Node> firstStandardAST = reader.readASTDump("../asts/first_standard_ast.txt");
    std::unique_ptr<Node> secondStandardAST = reader.readASTDump("../asts/second_standard_ast.txt");

    std::cout << firstStandardAST->name << " " << firstStandardAST->value <<'\n';
    std::cout << secondStandardAST->name << " " << secondStandardAST->value << '\n';
}