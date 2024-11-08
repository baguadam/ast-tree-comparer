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
    std::vector<Node*> getNodesFromMap(const std::unordered_multimap<std::string, Node*>&, const std::string&) const;
    const std::vector<Node*> getDeclNodes(const std::string& nodeKey) const;
    const std::vector<Node*> getStmtNodes(const std::string& nodeKey) const;
    const std::unordered_map<std::string, Node*>& getDeclNodeMap() const;
    const std::unordered_multimap<std::string, Node*>& getMultiDeclNodeMap() const;
    const std::unordered_multimap<std::string, Node*>& getStmtNodeMultiMap() const;

    bool isDeclNodeInAST(const std::string&) const;
    void processSubTree(Node*, std::function<void(Node*, int)>);
private:
    Node* root;
    std::unordered_map<std::string, Node*> declNodeMap;
    std::unordered_multimap<std::string, Node*> multiDeclNodeMap;
    std::unordered_multimap<std::string, Node*> stmtNodeMultiMap;

    Node* buildTree(std::ifstream&);
    void addStmtNodeToNodeMap(Node*, const std::string&);
    void addDeclNodeToNodeMap(Node*);
    void deleteTree(Node*);
};

#endif