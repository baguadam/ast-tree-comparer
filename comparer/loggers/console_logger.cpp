#include <iostream>
#include "../headers/loggers/console_logger.h"
#include "./headers/utils.h"

void ConsoleLogger::logNode(const Node* node, const std::string& differencType, std::string indent) {
    std::cout << indent << "Node details:\n";
    std::cout << indent << node->kind << " " << node->type << " " << node->usr << " " << node->path << " " << node->lineNumber << ":" << node->columnNumber << "\n";
    std::cout << indent << "*** Parent unique id: " << (node->parent ? Utils::getKey(node->parent, node->parent->type == "Declaration") : "None") << "\n";
    
    logSeparators();
}

void ConsoleLogger::logEdge(const Node* child, const Node* parent, std::string indent) {
    std::cout << indent << "Edge: " << Utils::getKey(child, false) << " -> " << Utils::getKey(parent, true) << "\n";
    Utils::printSeparators();
}

void ConsoleLogger::logSeparators() {
    std::cout << "----------------------------------------\n";
}