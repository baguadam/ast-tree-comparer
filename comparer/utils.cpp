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

/*
Description:
    Generates a unique key for a statement node based on its child and parent
*/
std::string Utils::getStatementId(const Node* node, const Node* declarationParent) {
    if (declarationParent) {
        std::string parentKey = Utils::getKey(declarationParent, true);
        std::string statementKey = Utils::getKey(node, false);
        return parentKey + "|" + statementKey;  // Concatenate parent and current node's key
    } else {
        return Utils::getKey(node, node->type == DECLARATION);  // Fallback: if no declaration parent, use the node's key
    }
}

/*
Description:
    Finds the first declaration parent of a given node (most cases it is for Statement nodes)
*/
const Node* Utils::findDeclarationParent(const Node* node) {
    const Node* parent = node->parent;
    while (parent && parent->type != DECLARATION) {
        parent = parent->parent;
    }
    return parent;
}

/*
Description:
    Prints a separator line to the console
*/
void Utils::printSeparators() {
    std::cout << "----------------------------------------\n";
}

/*
Description:
    Prints the details of a node to the console
*/
void Utils::printNodeDetails(const Node* node, std::string indent) {
    std::cout << indent << "Node details:\n";
    std::cout << indent << node->kind << " " << node->type << " " << node->usr << " " << node->path << " " << node->lineNumber << ":" << node->columnNumber << "\n";
    std::cout << indent << "*** Parent unique id: " << (node->parent ? getKey(node->parent, node->parent->type == DECLARATION) : "None") << "\n";
    
    printSeparators();
}

/*
Description:
    Converts an AST ID to a string
*/
std::string Utils::astIdToString(ASTId ast) {
    switch (ast) {
        case FIRST_AST: return "FIRST AST";
        case SECOND_AST: return "SECOND AST";
        default: return "UNKNOWN AST";
    }
}

/*
Description:
    Converts a node type to a string
*/
std::string Utils::nodeTypeToString(NodeType type) {
    switch (type) {
        case DECLARATION: return "Declaration";
        case STATEMENT: return "Statement";
        default: return "Unknown";
    }
}

/*
Description:
    Converts a string to a node type
*/
NodeType Utils::stringToNodeType(const std::string& type) {
    if (type == "Declaration") {
        return DECLARATION;
    } else if (type == "Statement") {
        return STATEMENT;
    } else {
        return UNKNOWN;
    }
}

/*
Description:
    Converts a difference type to a string
*/
std::string Utils::differenceTypeToString(DifferenceType type) {
    switch (type) {
        case ONLY_IN_FIRST_AST: return "NODE EXISTS ONLY IN FIRST AST";
        case ONLY_IN_SECOND_AST: return "NODE EXISTS ONLY IN SECOND AST";
        case DIFFERENT_PARENT: return "NODES HAVE DIFFERENT PARENTS";
        case DIFFERENT_SOURCE_LOCATIONS: return "NODES HAVE DIFFERENT SOURCE LOCATIONS";
        default: return "UNKNOWN DIFFERENCE";
    }
}
