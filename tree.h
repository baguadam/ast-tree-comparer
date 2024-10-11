#ifndef _NODE_READER_H_

#define _NODE_READER_H_

#include <string>
#include <vector>

struct Node {
    std::string name;
    std::string value;
    std::vector<Node*> children;
    Node* parent;
};

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