#include <iostream>
#include "../headers/loggers/console_logger.h"
#include "../headers/utils.h"

void ConsoleLogger::logNode(const Node* node, const DifferenceType diffType, const ASTId ast, std::string indent) {
    std::cout << indent << "Node with difference type: " << Utils::differenceTypeToString(diffType) << "\n";
    Utils::printNodeDetails(node, indent);
    Utils::printSeparators();
}