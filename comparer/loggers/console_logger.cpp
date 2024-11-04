#include <iostream>
#include "./headers/loggers/console_logger.h"
#include "./headers/utils.h"

void ConsoleLogger::logNode(const Node* node, const std::string& differenceType, std::string indent) {
    std::cout << indent << "Node with difference type: " << differenceType << "\n";
    Utils::printNodeDetails(node, indent);
    Utils::printSeparators();
}

void ConsoleLogger::logEdge(const Node* child, const Node* parent, std::string indent) {
    std::cout << indent << "Child Node: " << Utils::getKey(child, child->type == "Declaration") 
              << " has parent Node: " << Utils::getKey(parent, parent->type == "Declaration") << "\n";
    Utils::printSeparators();
}