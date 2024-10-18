#ifndef _NODE_H_

#define _NODE_H_

#include <string>
#include <vector>

struct Node {
    std::string type; // (Declaration/Statement)
    std::string kind; // (FunctionDecl/VarDecl/IfStmt/WhileStmt/...)
    std::string usr;  // unique identifier (in case of decl types)
    std::string path; // source file
    int lineNumber;   // which line in the source file
    int columnNumber; // which column in the source file
    Node* parent;
    std::vector<Node*> children;
};

#endif