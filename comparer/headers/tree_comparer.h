#ifndef _TREE_COMPARER_H_

#define _TREE_COMPARER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "node.h"

class TreeComparer {
public:
    TreeComparer(Node*, Node*, std::unordered_map<std::string, std::pair<Node*, bool>>&, std::unordered_map<std::string, std::pair<Node*, bool>>&);
    void printDifferences();

private:
    Node* firstASTTree;
    Node* secondASTTree;
    std::unordered_map<std::string, std::pair<Node*, bool>>& nodeMapFirstAST;
    std::unordered_map<std::string, std::pair<Node*, bool>>& nodeMapSecondAST;

    void compareSourceLocations(Node*, Node*);
    void compareStatements(Node*, Node*);
    void compareDeclarations(Node*, Node*);
    void compareFunctions(Node*, Node*);
    void compareClasses(Node*, Node*);
    void compareNodes(Node*, Node*);
    void processNode(Node*);
    void processNodeInSingleAST(Node*, const std::string&, std::unordered_map<std::string, std::pair<Node*, bool>>&, const char*);
    void processNodeInFirstAST(Node*, const std::string&);
    void processNodeInSecondAST(Node*, const std::string&);
    void enqueueChildren(Node*, std::queue<Node*>&);
    void markSubTreeAsProcessed(Node*, std::unordered_map<std::string, std::pair<Node*, bool>>&);

    bool isNodeInAST(const std::string&, const std::unordered_map<std::string, std::pair<Node*, bool>>&) const;
    bool isNodeInFirstAST(const std::string&) const;
    bool isNodeInSecondAST(const std::string&) const;
    bool isNodeProcessedInAST(const std::string&, const std::unordered_map<std::string, std::pair<Node*, bool>>&) const;

    void printNodeDetails(Node*, const std::string) const;
    void printSubTree(Node*, int) const;
    void printSeparators() const;
};


#endif