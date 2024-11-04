#include "./headers/loggers/database_logger.h"

DatabaseLogger::DatabaseLogger(Database& db) : db(db) { }

void DatabaseLogger::logNode(const Node* node, const DifferenceType diffType, const ASTId ast, std::string indent) {
    db.insertNode(node, ast, diffType);

    if (node->parent) {
        db.insertEdge(node->id, node->parent->id);
    }
}