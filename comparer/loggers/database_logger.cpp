#include "./headers/loggers/database_logger.h"

DatabaseLogger::DatabaseLogger(Database& db) : db(db) { }

void DatabaseLogger::logNode(const Node* node, DifferenceType diffType, ASTId ast, std::string indent) {
    
}

void DatabaseLogger::logEdge(const Node* childId, const Node* parentId, std::string indent) {

}