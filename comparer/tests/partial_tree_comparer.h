#ifndef _PARTIAL_TREE_COMPARER_H_

#define _PARTIAL_TREE_COMPARER_H_
#include <gmock/gmock.h>
#include "../include/tree_comparer.h"
#include "../include/node.h"

class TreeComparerTestWrapper : public TreeComparer {
public:
    using TreeComparer::compareSourceLocations;
    using TreeComparer::compareParents;
    using TreeComparer::compareSimilarDeclNodes;
    using TreeComparer::processNodesInSingleAST;
    
    using TreeComparer::processDeclNodes;

    TreeComparerTestWrapper(Tree& t1, Tree& t2, IDatabaseWrapper& dbWrapper)
        : TreeComparer(t1, t2, dbWrapper) { }
};

class BaseMockTreeComparer : public TreeComparer {
public:
    BaseMockTreeComparer(Tree& firstTree, Tree& secondTree, IDatabaseWrapper& db)
        : TreeComparer(firstTree, secondTree, db) {}

    MOCK_METHOD(void, compareParents, (const Node* firstNode, const Node* secondNode), (override));
    MOCK_METHOD(void, compareSourceLocations, (const Node* firstNode, const Node* secondNode), (override));
    MOCK_METHOD(void, processNodesInSingleAST, (Node* current, Tree& tree, const ASTId ast, bool isDeclaration), (override));
};

class PartialMockTreeComparer : public BaseMockTreeComparer{
public:
    PartialMockTreeComparer(Tree& firstTree, Tree& secondTree, IDatabaseWrapper& db)
        : BaseMockTreeComparer(firstTree, secondTree, db) {}

    MOCK_METHOD(void, compareSimilarDeclNodes, (Node* firstNode, Node* secondNode), (override));

    using TreeComparer::compareStmtNodes;
    using TreeComparer::processMultiDeclNodes;
    using TreeComparer::processDeclNodesInBothASTs;
};

class PartialMockTreeComparerForDeclNodes : public BaseMockTreeComparer {
public:
    PartialMockTreeComparerForDeclNodes(Tree& firstTree, Tree& secondTree, IDatabaseWrapper& db)
        : BaseMockTreeComparer(firstTree, secondTree, db) {}

    MOCK_METHOD(void, compareStmtNodes, (const Node* firstNode, const Node* secondNode), (override));
    MOCK_METHOD(void, processDeclNodesInBothASTs, (const std::string&), (override));

    using TreeComparer::processDeclNodes;
    using TreeComparer::compareSimilarDeclNodes;
};

#endif