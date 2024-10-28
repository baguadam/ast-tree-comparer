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

struct NodeHash {
    std::size_t operator()(const Node* node) const {
        return std::hash<std::string>{}(node->kind + node->usr + node->path + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber));
    }
};

struct NodeEqual {
    bool operator()(const Node* lhs, const Node* rhs) const {
        return lhs->kind == rhs->kind && lhs->usr == rhs->usr && lhs->path == rhs->path;
    }
};

#endif