#ifndef _DATABASE_LOGGER_H_

#define _DATABASE_LOGGER_H_

#include "tree_comparer_logger.h"
#include "../database.h"

class DatabaseLogger : public TreeComparerLogger { 
public:
    DatabaseLogger(Database& db);
    void logNode(const Node* node, const DifferenceType diffType, const ASTId ast, std::string indent = "") override;
    void logEdge(const Node* childId, const Node* parentId, std::string indent = "") override;

private:
    Database& db;
};

#endif