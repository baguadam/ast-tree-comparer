#include "./headers/utils.h"
/*
Description:
    Generates a unique key for a node based on its type and information
*/
std::string Utils::getKey(const Node* node, bool isDeclaration) {
    if (isDeclaration) {
        if (node->kind == "Typedef" || node->kind == "Field" || node->kind == "IntegralLiteral" || node->kind == "CXXRecord" || 
            node->kind == "TranslationUnit" || node->kind == "TemplateTypeParm" || node->kind == "ClassTemplate") {
            return node->kind + "|" + node->usr;
        } else {
            return node->kind + "|" + node->usr + "|" + node->path + "|" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber);
        }
    } else {
        return node->kind + "|" + node->path + "|" + getKey(node->parent, node->parent->type == "Declaration");  
    }
}