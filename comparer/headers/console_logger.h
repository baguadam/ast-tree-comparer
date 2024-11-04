#ifndef _CONSOLE_LOGGER_H_

#define _CONSOLE_LOGGER_H_

#include "tree_comparer_logger.h"

class ConsoleLogger : TreeComparerLogger { 
public:
    void logNode(const Node* node, const std::string& differencType, std::string indent = "") override;
    void logEdge(const Node* childId, const Node* parentId, std::string indent = "") override;

private:
    void logSeparators();
};

#endif