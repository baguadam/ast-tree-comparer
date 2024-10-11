#ifndef _TREE_COMPARER_H_

#define _TREE_COMPARER_H_

#include <string>
#include <unordered_map>
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

    std::unordered_map<std::string, Node*> createNodeMap(Node*);
};


#endif