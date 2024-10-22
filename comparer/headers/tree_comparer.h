#ifndef _TREE_COMPARER_H_

#define _TREE_COMPARER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "node.h"

class TreeComparer {
public:
    TreeComparer(Node*, Node*);
    void printDifferences();

private:
    Node* firstASTTree;
    Node* secondASTTree;
    std::unordered_map<std::string, std::pair<Node*, bool>> nodeMapFirstAST;
    std::unordered_map<std::string, std::pair<Node*, bool>> nodeMapSecondAST;
    std::string generateKey(Node*, bool);
    std::unordered_map<std::string, std::pair<Node*, bool>> createNodeMap(Node*);

    void compareSourceLocations(Node*, Node*);
    void compareStatements(Node*, Node*);
    void compareDeclarations(Node*, Node*);
    void compareNodes(Node*, Node*);
    void processNodeInFirstAST(Node*, const std::string&);
    void processNodeInSecondAST(Node*, const std::string&);
    void enqueueChildren(Node*, std::queue<Node*>&);
    void markSubTreeAsProcessed(Node*, std::unordered_map<std::string, std::pair<Node*, bool>>&);

    void printNodeDetails(Node*, const std::string);
    void printSubTree(Node*, int);
    void printSeparators();
};


#endif