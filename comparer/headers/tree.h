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
    const std::unordered_map<std::string, std::pair<Node*, bool>>& getNodeMap() const;
    
    void markNodeAsProcessed(const std::string& nodeKey);
    bool isNodeProcessed(const std::string& nodeKey) const;
    bool isNodeInAST(const std::string& nodeKey) const;
    const Node* getNodeFromNodeMap(const std::string& nodeKey) const;
    void markPairAsProcessed(const std::string& nodeKey);
    void markSubTreeAsProcessed(Node* node);

private:
    Node* root;
    std::unordered_map<std::string, std::pair<Node*, bool>> nodeMap;

    Node* buildTree(const std::string&);
    void createNodeMap();
    void deleteTree(Node* node);
};

#endif