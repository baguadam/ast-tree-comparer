#ifndef _DATABASE_LOGGER_H_

#define _DATABASE_LOGGER_H_

#include "tree_comparer_logger.h"
#include "../database.h"

class DatabaseLogger : public TreeComparerLogger { 
public:
    DatabaseLogger(Database&);
    void logNode(const Node*, const DifferenceType, const ASTId, std::string indent = "") override;
    void logEdge(const std::string&, const std::string&, std::string indent = "") override;

private:
    Database& db;
};

#endif