#ifndef _TREE_COMPARER_LOGGERS_H_

#define _TREE_COMPARER_LOGGERS_H_

#include <string>
#include "node.h"
#include "../enums.h"

class TreeComparerLogger {
public:
    virtual ~TreeComparerLogger() = default;
    virtual void logNode(const Node* node, const DifferenceType diffType, const ASTId ast, std::string indent = "") = 0;
    virtual void logEdge(const std::string&, const std::string&, std::string indent = "") = 0;
};

#endif