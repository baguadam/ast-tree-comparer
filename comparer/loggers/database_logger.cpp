#include "../headers/loggers/database_logger.h"
#include <iostream>

DatabaseLogger::DatabaseLogger(Database& db) : db(db) { }

void DatabaseLogger::logNode(const Node* node, const DifferenceType diffType, const ASTId ast, int depth) {
    try {
        bool isHighestLevelNode = (depth == 0);
        db.insertNode(node, ast, diffType, isHighestLevelNode);

        if (node->parent) {
            db.insertEdge(node->id, node->parent->id);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error logging node: " << e.what() << std::endl;
    }
}