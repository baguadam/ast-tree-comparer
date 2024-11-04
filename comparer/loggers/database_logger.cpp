#include "./headers/loggers/database_logger.h"

DatabaseLogger::DatabaseLogger(Database& db) : db(db) { }

void DatabaseLogger::logNode(const Node* node, const DifferenceType diffType, const ASTId ast, std::string indent) {
    
}

void DatabaseLogger::logEdge(const Node* childId, const Node* parentId, std::string indent) {

}