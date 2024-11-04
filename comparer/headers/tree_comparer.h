#ifndef _TREE_COMPARER_H_

#define _TREE_COMPARER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "node.h"
#include "tree.h"
#include "./loggers/tree_comparer_logger.h"

class TreeComparer {
public:
    TreeComparer(Tree&, Tree&, std::unique_ptr<TreeComparerLogger>);
    void printDifferences();

private:
    Tree& firstASTTree;
    Tree& secondASTTree;
    std::unique_ptr<TreeComparerLogger> logger;

    void compareSourceLocations(const Node*, const Node*);
    void compareParents(const Node*, const Node*);
    void compareSimilarDeclNodes(const Node*, const Node*, const std::string&);
    void compareStmtNodes(const std::string&);
    void compareSimilarStmtNodes(const Node*, const Node*);
    void processDeclNode(Node*);
    void processNodeInSingleAST(Node*, Tree&, const char*, bool, std::unordered_set<std::string>* processedKeys = nullptr);
    void enqueueChildren(Node*, std::queue<Node*>&);
};

#endif