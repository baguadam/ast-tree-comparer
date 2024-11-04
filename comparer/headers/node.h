#ifndef _NODE_H_

#define _NODE_H_

#include <string>
#include <vector>
#include "enums.h"

struct Node {
    std::string id;   // unique identifier
    NodeType type;    // (Declaration/Statement)
    std::string kind; // (FunctionDecl/VarDecl/IfStmt/WhileStmt/...)
    std::string usr;  // usr
    std::string path; // source file
    int lineNumber;   // which line in the source file
    int columnNumber; // which column in the source file
    Node* parent;
    std::vector<Node*> children;
    bool isProcessed = false; // flag for processed nodes
};

#endif