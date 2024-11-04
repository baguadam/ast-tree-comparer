#include "./headers/loggers/database_logger.h"

DatabaseLogger::DatabaseLogger(Database& db) : db(db) { }

void DatabaseLogger::logNode(const Node* node, const DifferenceType diffType, const ASTId ast, std::string indent) {
    db.insertNode(node, ast, diffType);
}

void DatabaseLogger::logEdge(const std::string& childId, const std::string& parentId, std::string indent) {
    db.insertEdge(childId, parentId);
}