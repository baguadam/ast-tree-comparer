#ifndef _TREE_COMPARER_H_

#define _TREE_COMPARER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <queue>
#include "node.h"
#include "tree.h"
#include "neo4j_database_wrapper.h"
#include "idatabase_wrapper.h"

class TreeComparer {
public:
    TreeComparer(Tree&, Tree&, IDatabaseWrapper&);
    void printDifferences();

protected:
    Tree& firstASTTree;
    Tree& secondASTTree;
    IDatabaseWrapper& dbWrapper;
    std::function<bool(const Node*, const Node*)> topologicalComparer;

    virtual void compareSourceLocations(const Node*, const Node*);
    virtual void compareParents(const Node*, const Node*);
    virtual void compareSimilarDeclNodes(Node*, Node*);
    virtual void compareStmtNodes(const Node*, const Node*);
    virtual void processNodesInSingleAST(Node*, Tree&, const ASTId, bool);
    void processMultiDeclNodes(const std::pair<std::unordered_multimap<std::string, Node*>::const_iterator,
                                               std::unordered_multimap<std::string, Node*>::const_iterator>&,
                               const std::pair<std::unordered_multimap<std::string, Node*>::const_iterator,
                                               std::unordered_multimap<std::string, Node*>::const_iterator>&);
    virtual void processDeclNodesInBothASTs(const std::string&);
    void processDeclNodes(Node*);
    void processRemainingNodes(std::vector<Node*>::const_iterator, std::vector<Node*>::const_iterator, Tree&, const ASTId);
    void enqueueChildren(Node*, std::queue<Node*>&);
};

#endif