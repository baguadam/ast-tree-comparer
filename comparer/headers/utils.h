#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include "node.h"
#include "enums.h"

class Utils {
public:
    static std::string getKey(const Node*, bool);
    static std::string getStatementId(const Node*, const Node*);
    static const Node* findDeclarationParent(const Node*);
    static void printSeparators();
    static void printNodeDetails(const Node*, std::string);

    static std::string astIdToString(const ASTId);
    static std::string nodeTypeToString(const NodeType);
    static NodeType stringToNodeType(const std::string&);
    static std::string differenceTypeToString(const DifferenceType);
};

#endif 