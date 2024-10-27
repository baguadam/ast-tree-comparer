#include ".headers/utils.h"
/*
Description:
    Generates a unique key for a node based on its type and information
*/
std::string Utils::getKey(Node* node, bool isDeclaration) const {
    std::string nodeKey = node->kind + "|" + node->usr;
    // for function it's important to make the key unique by adding the source location in the code
    if (node->kind == "Function") {
        nodeKey += "|" + node->path + "|" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber);
    } else if (!isDeclaration) { 
        // for statements we use the kind, path, line and column
        nodeKey += "|" + node->kind + "|" + node->path + "|" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber) + "|" + node->parent->usr; 
    }

    return nodeKey;
}