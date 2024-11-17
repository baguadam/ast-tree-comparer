#ifndef _NODE_H_

#define _NODE_H_

#include <string>
#include <vector>
#include "enums.h"
struct Node {
    // Node properties
    NodeType type;                   // Declaration/Statement
    std::string kind;                // FunctionDecl/VarDecl/IfStmt/WhileStmt/...
    std::string usr;                 // USR
    std::string path;                // Source file path
    int lineNumber;                  // Which line in the source file
    int columnNumber;                // Which column in the source file
    int topologicalOrder = -1;       // Topological order of the node

    // Relationships
    Node* parent = nullptr;          // Parent node pointer
    std::vector<Node*> children;     // Children of this node

    // Unique properties and flags
    std::string enhancedKey;         // Identifier (path from the root)
    size_t fingerprint = 0;          // Hashed fingerprint value of the node (only for declaration nodes)
    bool isProcessed = false;        // Flag for processed nodes

};

#endif