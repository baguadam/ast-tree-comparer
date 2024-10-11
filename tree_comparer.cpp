#include <iostream>
#include "tree.h"

void printDifferences(Node* firstAST, Node* secondAST, const std::string& prefix = "") {
    if (firstAST == nullptr || secondAST == nullptr) {
        return;
    }
    if (firstAST == nullptr) {
        std::cout << prefix << "Node added in second AST: " << secondAST->name << " " << secondAST->value << '\n';
        for (Node* child : secondAST->children) {
            printDifferences(nullptr, child, prefix + " ");
        }
        return;
    }
    if (secondAST == nullptr) {
        std::cout << prefix << "Node removed from first AST " << firstAST->name << " " << firstAST->value << '\n';
        for (Node* child : firstAST->children) {
            printDifferences(child, nullptr, prefix + " ");
        } 
        return;
    }
    if (firstAST->name != secondAST->name || firstAST->value != secondAST->value) {
        std::cout << prefix << "Node changed from: " << firstAST->name << " " << firstAST->value << " to: "
                  << secondAST->name << " " << secondAST->value << '\n';
    }
    for (size_t i = 0; i < std::max(firstAST->children.size(), secondAST->children.size()); ++i) {
        Node* firstChild = i < firstAST->children.size() ? firstAST->children[i] : nullptr;
        Node* secondChild = i < secondAST->children.size() ? secondAST->children[i] : nullptr;

        printDifferences(firstChild, secondChild, prefix + " ");
    }
}

int main() {
    Tree firstStandardAST("../asts/first_standard_ast.txt");
    Tree secondStandardAST("../asts/second_standard_ast.txt");

    std::cout << firstStandardAST.getRoot()->name << " " << firstStandardAST.getRoot()->value << '\n';
    std::cout << secondStandardAST.getRoot()->name << " " << secondStandardAST.getRoot()->value << '\n';

    printDifferences(firstStandardAST.getRoot(), secondStandardAST.getRoot());
}   