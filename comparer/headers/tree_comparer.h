#ifndef _TREE_COMPARER_H_

#define _TREE_COMPARER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "node.h"
#include "tree.h"

class TreeComparer {
public:
    TreeComparer(Tree&, Tree&);
    void printDifferences();

private:
    Tree& firstASTTree;
    Tree& secondASTTree;

    void compareSourceLocations(const Node*, const Node*);
    void compareStatements(const Node*, const Node*);
    void compareDeclarations(const Node*, const Node*);
    void compareFunctions(const Node*, const Node*);
    void compareNodes(const Node*, const Node*);
    void processNode(Node*);
    void processNodeInSingleAST(Node*, const std::string&, Tree&, const char*);
    void processNodeInFirstAST(Node*, const std::string&);
    void processNodeInSecondAST(Node*, const std::string&);
    void enqueueChildren(Node*, std::queue<Node*>&);

    void printNodeDetails(const Node*, const std::string) const;
    void printSubTree(const Node*, int) const;
    void printSeparators() const;
};

#endif