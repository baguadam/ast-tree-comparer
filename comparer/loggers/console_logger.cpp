#include <iostream>
#include "../headers/loggers/console_logger.h"
#include "../headers/utils.h"

void ConsoleLogger::logNode(const Node* node, const std::string& differenceType, std::string indent) {
    std::cout << indent << "Node with difference type: " << differenceType << "\n";
    Utils::printNodeDetails(node, indent);
    Utils::printSeparators();
}

void ConsoleLogger::logEdge(const Node* childId, const Node* parentId, std::string indent) { }