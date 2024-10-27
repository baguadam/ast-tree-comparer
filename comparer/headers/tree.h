#ifndef _TREE_H_

#define _TREE_H_

#include <string>
#include <unordered_map>
#include "node.h"

class Tree {
public:
    Tree(const std::string&);
    ~Tree();
    Node* getRoot() const;
    std::unordered_map<std::string, std::pair<Node*, bool>>& getNodeMap();

private:
    Node* root;
    std::unordered_map<std::string, std::pair<Node*, bool>> nodeMap;

    Node* buildTree(const std::string&);
    void createNodeMap();
    void deleteTree(Node* node);
};

#endif