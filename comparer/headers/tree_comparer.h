#ifndef _TREE_COMPARER_H_

#define _TREE_COMPARER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "node.h"

class TreeComparer {
public:
    TreeComparer(Node*, Node*);
    void printDifferences();

private:
    Node* firstASTTree;
    Node* secondASTTree;
    std::unordered_map<std::string, Node*> nodeMapFirstAST;
    std::unordered_map<std::string, Node*> nodeMapSecondAST;
    std::string generateKey(Node*, bool);
    std::unordered_map<std::string, Node*> createNodeMap(Node*);

    void compareSourceLocations(Node*, Node*);
    void compareStatements(Node*, Node*);
    void compareDeclarations(Node*, Node*);
    void compareNodes(Node*, Node*);
    void printNodeDetails(Node*, std::string);
    void printSubTree(Node*, int);
    void printSeparators();
    void markSubTreeAsProcessed(Node*, std::unordered_set<std::string>&);
};


#endif