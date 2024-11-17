#include <iostream>
#include "../include/loggers/console_logger.h"
#include "../include/utils.h"

void ConsoleLogger::logNode(const Node* node, const DifferenceType diffType, const ASTId ast, int depth) {
    std::string indent(depth * 3, ' ');
    
    std::cout << indent << "Node with difference type: " << Utils::differenceTypeToString(diffType) << "\n";
    Utils::printNodeDetails(node, indent);
    Utils::printSeparators();
}