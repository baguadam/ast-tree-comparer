#include "node_reader.h"

int main() {
    NodeReader reader;

    Node* firstStandardAST = reader.readASTDump("first_standard_ast.txt");
}