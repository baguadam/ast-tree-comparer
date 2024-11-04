#ifndef _TREE_COMPARER_LOGGERS_H_

#define _TREE_COMPARER_LOGGERS_H_

#include <string>
#include "node.h"

class TreeComparerLogger {
public:
    virtual ~TreeComparerLogger() = default;
    virtual void logNode(const Node* node, const std::string& differencType, std::string indent = "") = 0;
    virtual void logEdge(const Node* childId, const Node* parentId, std::string indent = "") = 0;
};

#endif