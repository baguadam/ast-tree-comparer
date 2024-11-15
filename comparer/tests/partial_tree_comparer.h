#ifndef _PARTIAL_TREE_COMPARER_H_

#define _PARTIAL_TREE_COMPARER_H_
#include <gmock/gmock.h>
#include "../headers/tree_comparer.h"
#include "../headers/node.h"

class PartialMockTreeComparer : public TreeComparer {
public:
    PartialMockTreeComparer(Tree& firstTree, Tree& secondTree, IDatabaseWrapper& db)
        : TreeComparer(firstTree, secondTree, db) {}

    MOCK_METHOD(void, compareSimilarDeclNodes, (Node* firstNode, Node* secondNode), (override));
    MOCK_METHOD(void, compareSimilarStmtNodes, (Node* firstNode, Node* secondNode), (override));
    MOCK_METHOD(void, processNodesInSingleAST, (Node* current, Tree& tree, const ASTId ast, bool isDeclaration), (override));

    using TreeComparer::compareStmtNodes;
    using TreeComparer::processMultiDeclNodes;
    using TreeComparer::processDeclNodesInBothASTs;
};

#endif