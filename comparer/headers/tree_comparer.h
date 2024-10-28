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
    void compareParents(const Node*, const Node*);
    void compareSimilarDeclNodes(const Node*, const Node*, const std::string&);
    void compareStmtNodes(const std::string&);
    void compareSimilarStmtNodes(const Node*, const Node*);
    void processDeclNode(Node*);
    void processDeclNodeInSingleAST(Node*, const std::string&, Tree&, const char*);
    void processDeclNodeInFirstAST(Node*, const std::string&);
    void processDeclNodeInSecondAST(Node*, const std::string&);
    void enqueueChildren(Node*, std::queue<Node*>&);

    void printNodeDetails(const Node*, const std::string) const;
    void printSubTree(const Node*, int) const;
    void printSeparators() const;
};

#endif