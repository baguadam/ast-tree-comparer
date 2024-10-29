#include "./headers/utils.h"
#include <iostream>
/*
Description:
    Generates a unique key for a node based on its type and information
*/
std::string Utils::getKey(const Node* node, bool isDeclaration) {
    std::string key = node->kind;

    if (isDeclaration) {
        if (node->kind == "Typedef" || node->kind == "Field" || node->kind == "IntegralLiteral" || node->kind == "CXXRecord" || 
            node->kind == "TranslationUnit" || node->kind == "TemplateTypeParm" || node->kind == "ClassTemplate") {
            key += "|" + node->usr;
        } else {
            key += "|" + node->usr + "|" + node->path + "|" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber);
        }
    } else {
        key += "|" + node->usr + "|" + node->path + "|" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber);
    }

    return key;
}

const Node* Utils::findDeclarationParent(const Node* node) {
    const Node* parent = node->parent;
    while (parent && parent->type != "Declaration") {
        parent = parent->parent;
    }
    return parent;
}

void Utils::printSeparators() {
    std::cout << "----------------------------------------\n";
}

void Utils::printNodeDetails(const Node* node, std::string indent) {
    std::cout << indent << "Node details:\n";
    std::cout << indent << node->kind << " " << node->type << " " << node->usr << " " << node->path << " " << node->lineNumber << ":" << node->columnNumber << "\n";
    std::cout << indent << "*** Parent unique id: " << (node->parent ? getKey(node->parent, node->parent->type == "Declaration") : "None") << "\n";
    
    printSeparators();
}