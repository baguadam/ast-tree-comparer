#ifndef _TREE_H_

#define _TREE_H_

#include <string>
#include "node.h"

class Tree {
public:
    Tree(const std::string&);
    ~Tree();
    Node* getRoot() const;

private:
    Node* root;

    Node* buildTree(const std::string&);
    void deleteTree(Node* node);
};

#endif