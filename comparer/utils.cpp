#include "./headers/utils.h"
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
        const Node* declarationParent = findDeclarationParent(node);
        key += "|" + declarationParent->usr + "|" + declarationParent->path + "|" + std::to_string(declarationParent->lineNumber) + ":" + std::to_string(declarationParent->columnNumber);  
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