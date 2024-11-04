#ifndef _CONSOLE_LOGGER_H_

#define _CONSOLE_LOGGER_H_

#include "tree_comparer_logger.h"

class ConsoleLogger : public TreeComparerLogger { 
public:
    void logNode(const Node* node, const DifferenceType diffType, const ASTId ast, std::string indent = "") override;
    void logEdge(const Node* childId, const Node* parentId, std::string indent = "") override;
};

#endif