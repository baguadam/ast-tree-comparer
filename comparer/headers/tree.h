#ifndef _TREE_H_

#define _TREE_H_

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include "node.h"

class Tree {
public:
    Tree(const std::string&);
    ~Tree();
    
    Node* getRoot() const;
    const std::unordered_map<std::string, std::pair<Node*, bool>>& getDeclNodeMap() const;
    const std::unordered_multimap<std::string, Node*>& getStmtNodeMultiMap() const;
    const Node* getDeclNode(const std::string&) const;
    const std::vector<std::pair<std::string, Node*>> getStmtNodes(const std::string&) const;
    
    void markDeclNodeAsProcessed(const std::string&);
    bool isDeclNodeProcessed(const std::string&) const;
    bool isDeclNodeInAST(const std::string&) const;
    void markPairAsProcessed(const std::string&);
    void markDeclSubTreeAsProcessed(Node*);
    void markStmtSubTreeAsProcessed(Node*, std::unordered_set<std::string>&);

    static void markStmtSubTreeAsProcessed(Node*, std::unordered_map<std::string, std::pair<Node*, bool>>&);

private:
    Node* root;
    std::unordered_map<std::string, std::pair<Node*, bool>> declNodeMap;
    std::unordered_multimap<std::string, Node*> stmtNodeMultiMap;

    Node* buildTree(std::ifstream&);
    void createNodeMap();
    void traverseSubTree(Node*, std::function<void(Node*)> processNode);
    void deleteTree(Node*);
};

#endif