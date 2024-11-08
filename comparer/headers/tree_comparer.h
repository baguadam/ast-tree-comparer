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
    void compareSimilarDeclNodes(Node*, Node*, const std::string&);
    void compareStmtNodes(const std::string&);
    void compareSimilarStmtNodes(Node*, Node*);
    void processDeclNodes(Node*);
    void processDeclNodesInBothASTs(const std::string&);
    void processMultiDeclNodes(const std::pair<std::unordered_multimap<std::string, Node*>::const_iterator,
                                               std::unordered_multimap<std::string, Node*>::const_iterator>&,
                               const std::pair<std::unordered_multimap<std::string, Node*>::const_iterator,
                                               std::unordered_multimap<std::string, Node*>::const_iterator>&,
                               const std::string&);
    void processNodesInSingleAST(Node*, Tree&, const ASTId, bool);
    void processRemainingNodes(std::vector<Node*>::const_iterator, std::vector<Node*>::const_iterator, Tree&, const ASTId);
    void checkNodeFingerprints(Node*, Node*, const std::string&);
    
    void enqueueChildren(Node*, std::queue<Node*>&);
};

#endif