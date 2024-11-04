#ifndef _DATABASE_LOGGER_H_

#define _DATABASE_LOGGER_H_

#include "tree_comparer_logger.h"

class DatabaseLogger : public TreeComparerLogger { 
public:
    void logNode(const Node* node, DifferenceType diffType, ASTId ast, std::string indent = "") override;
    void logEdge(const Node* childId, const Node* parentId, std::string indent = "") override;
};

#endif