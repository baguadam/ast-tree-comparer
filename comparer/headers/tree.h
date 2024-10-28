#ifndef _TREE_H_

#define _TREE_H_

#include <string>
#include <unordered_set>s
#include <unordered_map>
#include "node.h"

class Tree {
public:
    Tree(const std::string&);
    ~Tree();
    
    Node* getRoot() const;
    const std::unordered_map<std::string, std::pair<Node*, bool>>& getDeclNodeMap() const;
    const std::unordered_multimap<std::string, Node*>& getStmtNodeMultiMap() const;
    const Node* getDeclNode(const std::string& nodeKey) const;
    const std::unordered_set<Node*> getStmtNodes(const std::string& nodeKey) const;
    
    void markDeclNodeAsProcessed(const std::string& nodeKey);
    bool isDeclNodeProcessed(const std::string& nodeKey) const;
    bool isDeclNodeInAST(const std::string& nodeKey) const;
    void markPairAsProcessed(const std::string& nodeKey);
    void markSubTreeAsProcessed(Node* node);

private:
    Node* root;
    std::unordered_map<std::string, std::pair<Node*, bool>> declNodeMap;
    std::unordered_multimap<std::string, Node*> stmtNodeMultiMap;

    Node* buildTree(const std::string&);
    void createNodeMap();
    void deleteTree(Node* node);
};

#endif