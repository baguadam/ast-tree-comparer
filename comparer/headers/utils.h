#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include "node.h"
#include "enums.h"

class Utils {
public:
    static std::string getKey(const Node* node, bool isDeclaration);
    static const Node* findDeclarationParent(const Node* node);
    static void printSeparators();
    static void printNodeDetails(const Node* node, std::string indent);

    static std::string astIdToString(ASTId ast);
    static std::string nodeTypeToString(NodeType type);
    static std::string differenceTypeToString(DifferenceType type);
};

#endif 